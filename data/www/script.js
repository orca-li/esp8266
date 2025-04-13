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
});

manualTemperatureInput.addEventListener("input", function () {
    manualTemperatureRange.value = manualTemperatureInput.value;
    disableAutomaticMode();
    updateSetTemperature(manualTemperatureInput.value);
});

useAutomaticTemperatureButton.addEventListener("click", function () {
    automaticMode = !automaticMode;
    useAutomaticTemperatureButton.classList.toggle('active');

    if (automaticMode) {
        updateSetTemperature(automaticTemperatureDisplay.textContent.replace('°C', ''));
    }
});

function simulateAutomaticTemperature() {
    setInterval(function () {
        const temp = Math.floor(Math.random() * 61) - 30;
        automaticTemperatureDisplay.textContent = temp + "°C";
        if (automaticMode) {
          updateSetTemperature(temp);
        }
    }, 3000);
}

simulateAutomaticTemperature();
updateSetTemperature(manualTemperatureRange.value);