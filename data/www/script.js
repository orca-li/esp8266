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

simulateAutomaticTemperature();
updateSetTemperature(manualTemperatureRange.value);