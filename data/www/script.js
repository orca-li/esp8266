const manualTemperatureRange = document.getElementById("manualTemperatureRange");
const manualTemperatureInput = document.getElementById("manualTemperatureInput");
const automaticTemperatureDisplay = document.getElementById("automaticTemperature");
const setTemperatureDisplay = document.getElementById("setTemperature");
const useAutomaticTemperatureButton = document.getElementById("useAutomaticTemperature");

let automaticMode = false;

function updateSetTemperature(temperature) {
    setTemperatureDisplay.textContent = temperature + "°C";
}

function disableAutomaticMode() {
    automaticMode = false;
    useAutomaticTemperatureButton.classList.remove('active');
}

manualTemperatureRange.addEventListener("input", function () {
    manualTemperatureInput.value = manualTemperatureRange.value;
    disableAutomaticMode();
    updateSetTemperature(manualTemperatureRange.value);
    sendManualTemperature(manualTemperatureInput.value);
});

manualTemperatureInput.addEventListener("input", function () {
    manualTemperatureRange.value = manualTemperatureInput.value;
    disableAutomaticMode();
    updateSetTemperature(manualTemperatureInput.value);
    sendManualTemperature(manualTemperatureInput.value);
});

useAutomaticTemperatureButton.addEventListener("click", function () {
    automaticMode = !automaticMode;
    useAutomaticTemperatureButton.classList.toggle('active');

    if (automaticMode) {
        updateSetTemperature(automaticTemperatureDisplay.textContent.replace('°C', ''));
        sendAutoSensorMode();
    }
});

function simulateAutomaticTemperature() {
    setInterval(function () {
        fetch('/www/termo.json')
            .then(response => {
                if (!response.ok) throw new Error('Network error');
                return response.json();
            })
            .then(data => {
                const temp = data.temperature.toFixed(2);
                automaticTemperatureDisplay.textContent = temp + "°C";
                if (automaticMode) updateSetTemperature(temp);
            })
            .catch(error => {
                console.error('Error:', error);
                automaticTemperatureDisplay.textContent = "Error";
            });
    }, 3000);
}

function sendManualTemperature(temperature) {
    fetch('/setTemperature', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ manualTemperature: parseFloat(temperature) })
    })
        .then(response => {
            if (!response.ok) throw new Error('Failed to set temperature');
            console.log('Temperature sent successfully');
        })
        .catch(error => console.error('Error sending temperature:', error));
}

function sendAutoSensorMode() {
    fetch('/setMode', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mode: "auto" })
    })
        .then(response => {
            if (!response.ok) throw new Error('Failed to set auto mode');
            console.log('Auto mode sent successfully');
        })
        .catch(error => console.error('Error sending auto mode:', error));
}

simulateAutomaticTemperature();
updateSetTemperature(manualTemperatureRange.value);