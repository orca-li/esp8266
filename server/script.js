document.addEventListener('DOMContentLoaded', function() {
  // Элементы интерфейса
  const loginForm = document.getElementById('loginForm');
  const usernameInput = document.getElementById('usernameInput');
  const passwordInput = document.getElementById('passwordInput');
  const loginPanel = document.getElementById('login-panel');
  const mainContent = document.getElementById('main-content');
  const currentUsernameSpan = document.getElementById('currentUsername');
  const currentBrokerSpan = document.getElementById('currentBroker');
  const statusIndicator = document.getElementById('statusIndicator');
  const connectionStatusText = document.getElementById('connectionStatusText');
  const manualTemperatureRange = document.getElementById('manualTemperatureRange');
  const temperatureValue = document.getElementById('temperatureValue');
  const automaticTemperatureDisplay = document.getElementById('automaticTemperature');
  const useAutomaticTemperatureButton = document.getElementById('useAutomaticTemperature');
  const currentTemperatureSetDisplay = document.getElementById('currentTemperatureSet');
  const notificationPanel = document.getElementById('notificationPanel');
  const brokerOptions = document.querySelectorAll('input[name="broker"]');
  const customBrokerSection = document.getElementById('custom-broker');
  const brokerIpInput = document.getElementById('broker-ip');
  const brokerPortInput = document.getElementById('broker-port');

  // Состояние приложения
  let mqttClient;
  let automaticMode = false;
  let currentManualTemperature = 20.0;
  let currentSensorTemperature = null;
  let currentUsername = '';
  let currentBroker = '';
  let connectionCheckInterval;
  let lastMessageTime = 0;
  let reconnectAttempts = 0;
  const MAX_RECONNECT_ATTEMPTS = 3;
  let debounceTimer;
  let lastModeChangeTime = 0;
  let lastManualTempSent = null;
  let lastSensorTempSent = null;

  // Брокеры по умолчанию
  const BROKERS = {
    default: { ip: '5.35.126.101', port: '9001' },
    local: { ip: 'localhost', port: '1883' },
    custom: { ip: '', port: '' }
  };

  // Показ уведомления
  function showNotification(message, type = 'info', duration = 5000) {
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.innerHTML = `
      <span>${message}</span>
      <button class="notification-close">&times;</button>
    `;

    notification.querySelector('.notification-close').addEventListener('click', () => {
      notification.remove();
    });

    notificationPanel.appendChild(notification);

    if (duration > 0) {
      setTimeout(() => {
        notification.remove();
      }, duration);
    }
  }

  // Обработка ошибок
  function handleError(error, context = '') {
    console.error(`${context}:`, error);
    showNotification(`${context}: ${error.message || error}`, 'error');
  }

  // Обновление статуса подключения
  function updateConnectionStatus(connected) {
    if (connected) {
      statusIndicator.classList.add('connected');
      statusIndicator.classList.remove('disconnected');
      connectionStatusText.textContent = 'Connected';
      connectionStatusText.style.color = 'var(--success)';
      reconnectAttempts = 0;
      showNotification('Successfully connected to broker', 'success');
    } else {
      statusIndicator.classList.remove('connected');
      statusIndicator.classList.add('disconnected');
      connectionStatusText.textContent = 'Disconnected';
      connectionStatusText.style.color = 'var(--error)';
    }
  }

  // Проверка активности соединения
  function checkConnection() {
    if (mqttClient && mqttClient.connected) {
      const now = Date.now();
      if (now - lastMessageTime > 15000) {
        showNotification('No messages received for 15 seconds', 'warning');
        mqttClient.end();
      }
    }
  }

  // Обновление отображения температур
  function updateTemperatureDisplays() {
    temperatureValue.textContent = currentManualTemperature.toFixed(1) + "°C";
    manualTemperatureRange.value = currentManualTemperature;

    automaticTemperatureDisplay.textContent = currentSensorTemperature !== null ?
      currentSensorTemperature.toFixed(1) + "°C" : "N/A";

    currentTemperatureSetDisplay.textContent = automaticMode ?
      (currentSensorTemperature !== null ? currentSensorTemperature.toFixed(1) + "°C" : "N/A") :
      currentManualTemperature.toFixed(1) + "°C";
  }

  // Обновление режима работы
  function updateModeDisplay() {
    if (automaticMode) {
      useAutomaticTemperatureButton.classList.remove('manual');
      useAutomaticTemperatureButton.classList.add('automatic');
      useAutomaticTemperatureButton.textContent = 'USE MANUAL MODE';
      showNotification('Switched to automatic mode', 'info');
    } else {
      useAutomaticTemperatureButton.classList.remove('automatic');
      useAutomaticTemperatureButton.classList.add('manual');
      useAutomaticTemperatureButton.textContent = 'USE AUTOMATIC MODE';
      showNotification('Switched to manual mode', 'info');
    }
    updateTemperatureDisplays();
  }

  // Выбор брокера
  brokerOptions.forEach(option => {
    option.addEventListener('change', function() {
      if (this.value === 'custom') {
        customBrokerSection.style.display = 'flex';
      } else {
        customBrokerSection.style.display = 'none';
      }
    });
  });

  // Подключение к MQTT
  function connectMQTT(username, password) {
    const selectedBroker = document.querySelector('input[name="broker"]:checked').value;
    let brokerConfig = BROKERS[selectedBroker];

    if (selectedBroker === 'custom') {
      brokerConfig = {
        ip: brokerIpInput.value.trim(),
        port: brokerPortInput.value.trim()
      };
    }

    if (!brokerConfig.ip || !brokerConfig.port) {
      showNotification('Please enter valid broker address', 'error');
      return;
    }

    currentBroker = `${brokerConfig.ip}:${brokerConfig.port}`;
    currentBrokerSpan.textContent = currentBroker;

    const options = {
      username: username,
      password: password,
      clientId: 'thermostat_' + Math.random().toString(36).substr(2, 8),
      keepalive: 10,
      reconnectPeriod: 5000,
      protocolVersion: 5
    };

    if (mqttClient) {
      mqttClient.end();
    }

    showNotification(`Connecting to ${currentBroker}...`, 'info');

    try {
      mqttClient = mqtt.connect(`ws://${brokerConfig.ip}:${brokerConfig.port}`, options);

      mqttClient.on('connect', () => {
        console.log('Connected to MQTT Broker');
        mqttClient.subscribe(['esp8266/sensor_temp', 'esp8266/status'], { qos: 1 });
        sendAutoMode(automaticMode);
        updateModeDisplay();
        updateConnectionStatus(true);
        currentUsernameSpan.textContent = username;

        loginPanel.style.display = 'none';
        mainContent.style.display = 'block';

        lastMessageTime = Date.now();
        clearInterval(connectionCheckInterval);
        connectionCheckInterval = setInterval(checkConnection, 5000);
      });

      mqttClient.on('message', (topic, message) => {
        lastMessageTime = Date.now();

        if (topic === 'esp8266/sensor_temp') {
          const newSensorTemp = parseFloat(message.toString());
          if (newSensorTemp !== currentSensorTemperature) {
            currentSensorTemperature = newSensorTemp;
            updateTemperatureDisplays();

            // В автоматическом режиме отправляем значение датчика
            if (automaticMode && currentSensorTemperature !== null &&
                currentSensorTemperature !== lastSensorTempSent) {
              sendManualTemperature(currentSensorTemperature);
              lastSensorTempSent = currentSensorTemperature;
            }
          }
        } else if (topic === 'esp8266/status') {
          updateConnectionStatus(message.toString() === '1');
        }
      });

      mqttClient.on('error', (error) => {
        handleError(error, 'MQTT Error');
        updateConnectionStatus(false);
        reconnectAttempts++;

        if (reconnectAttempts <= MAX_RECONNECT_ATTEMPTS) {
          showNotification(`Reconnecting (attempt ${reconnectAttempts}/${MAX_RECONNECT_ATTEMPTS})`, 'warning');
        } else {
          showNotification('Max reconnection attempts reached', 'error');
          loginPanel.style.display = 'block';
          mainContent.style.display = 'none';
        }
      });

      mqttClient.on('close', () => {
        updateConnectionStatus(false);
        showNotification('Connection closed', 'warning');
      });

      mqttClient.on('offline', () => {
        updateConnectionStatus(false);
        showNotification('Connection offline', 'warning');
      });

      mqttClient.on('reconnect', () => {
        showNotification('Reconnecting...', 'info');
      });

    } catch (error) {
      handleError(error, 'Connection Error');
    }
  }

  // Обработчики событий с защитой от двойного нажатия
  manualTemperatureRange.addEventListener("input", function() {
    const newTemp = parseFloat(this.value);
    if (newTemp !== currentManualTemperature) {
      currentManualTemperature = newTemp;
      updateTemperatureDisplays();

      // Используем debounce для предотвращения множественных отправок
      clearTimeout(debounceTimer);
      debounceTimer = setTimeout(() => {
        if (!automaticMode && currentManualTemperature !== lastManualTempSent) {
          sendManualTemperature(currentManualTemperature);
          lastManualTempSent = currentManualTemperature;
        }
      }, 300);
    }
  });

  useAutomaticTemperatureButton.addEventListener("click", function() {
    const now = Date.now();
    // Защита от двойного нажатия (минимум 500мс между нажатиями)
    if (now - lastModeChangeTime < 500) return;
    lastModeChangeTime = now;

    automaticMode = !automaticMode;
    updateModeDisplay();

    // Отправляем только сообщение о режиме
    sendAutoMode(automaticMode);

    // Отправляем соответствующую температуру (если изменилась)
    if (automaticMode && currentSensorTemperature !== null &&
        currentSensorTemperature !== lastSensorTempSent) {
      sendManualTemperature(currentSensorTemperature);
      lastSensorTempSent = currentSensorTemperature;
    } else if (!automaticMode && currentManualTemperature !== lastManualTempSent) {
      sendManualTemperature(currentManualTemperature);
      lastManualTempSent = currentManualTemperature;
    }
  });

  // Отправка ручной температуры (используется и для автоматического режима)
  function sendManualTemperature(temperature) {
    if (mqttClient && mqttClient.connected) {
      try {
        mqttClient.publish('esp8266/manual_temp', temperature.toString(), { qos: 1 });
        console.log(`Sent temperature: ${temperature} (${automaticMode ? 'auto' : 'manual'} mode)`);
      } catch (error) {
        handleError(error, 'Send Error');
      }
    }
  }

  function sendAutoMode(enabled) {
    if (mqttClient && mqttClient.connected) {
      try {
        mqttClient.publish('esp8266/auto_mode', enabled ? "1" : "0", { qos: 1 });
        console.log(`Sent auto mode: ${enabled}`);
      } catch (error) {
        handleError(error, 'Mode Change Error');
      }
    }
  }

  // Обработчик формы входа
  loginForm.addEventListener('submit', function(e) {
    e.preventDefault();

    currentUsername = usernameInput.value.trim();
    const password = passwordInput.value.trim();

    if (currentUsername && password) {
      connectMQTT(currentUsername, password);
    } else {
      showNotification('Please enter both username and password', 'error');
    }
  });

  // Инициализация
  updateConnectionStatus(false);
  updateTemperatureDisplays();
});