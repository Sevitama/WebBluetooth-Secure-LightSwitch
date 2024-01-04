var deviceName = "WebBluetooth";
var interactiveServiceUUID = "b7e6e95f-b754-421a-93ef-efb5db537daa";
var ledCharacteristicUUID = "b7e6e95f-b754-421a-93ef-efb5db537da1";
var buttonCharacteristicUUID = "b7e6e95f-b754-421a-93ef-efb5db537db1";
var secureButtonCharacteristicUUID = "b7e6e95f-b754-421a-93ef-efb5db537dc1";
var bluetoothDeviceConnected = false;
var interactiveService;
var detectedBluetoothDevice;
var ledCharacteristic;
var buttonCharacteristic;
var secureButtonCharacteristic;

//---------------- Buttons ------------------------------

document.querySelector("#connect").addEventListener("click", function () {
    if (isWebBluetoothEnabled()) {
        connectDevice();
    }
});

document.querySelector("#on").addEventListener("click", function (event) {
    if (ledCharacteristic) {
        toggleLED(1);
    }
});

document.querySelector("#off").addEventListener("click", function (event) {
    if (ledCharacteristic) {
        toggleLED(0);
    }
});

document.querySelector("#start").addEventListener("click", function (event) {
    if (buttonCharacteristic) {
        toggleNotification(1);
    }
});

document.querySelector("#stop").addEventListener("click", function (event) {
    if (buttonCharacteristic) {
        toggleNotification(0);
    }
});

document.querySelector("#securestart").addEventListener("click", function (event) {
    if (secureButtonCharacteristic) {
        toggleSecureNotification(1);
    }
});

document.querySelector("#securestop").addEventListener("click", function (event) {
    if (secureButtonCharacteristic) {
        toggleSecureNotification(0);
    }
});

//---------------- Device Connections ------------------------------

function isWebBluetoothEnabled() {
    if (!navigator.bluetooth) {
        console.log("Web Bluetooth API is not available in this browser!");
        return false;
    }
    return true;
}

function connectDevice() {
    return (detectedBluetoothDevice ? Promise.resolve() : getDeviceInfo())
        .then(getService)
        .then(getCharacteristics)
        .catch((error) => {
            console.log("Waiting to start reading: " + error);
        });
}

function getDeviceInfo() {
    let options = {
        optionalServices: [interactiveServiceUUID],
        filters: [{ name: deviceName }],
    };

    console.log("Requesting any Bluetooth Device...");
    return navigator.bluetooth
        .requestDevice(options)
        .then((device) => {
            detectedBluetoothDevice = device;
        })
        .catch((error) => {
            console.log(error);
        });
}

function getService() {
    if (detectedBluetoothDevice.gatt.connected) {
        return Promise.resolve();
    }

    return detectedBluetoothDevice.gatt
        .connect()
        .then((server) => {
            console.log("Getting GATT Service...");
            return server.getPrimaryService(interactiveServiceUUID);
        })
        .then((service) => {
            interactiveService = service;
            console.log("Getting GATT Characteristic...");
        })
        .catch((error) => {
            detectedBluetoothDevice = null;
            console.log(error);
        });
}

async function getCharacteristics() {
    if (buttonCharacteristic && ledCharacteristic && secureButtonCharacteristic) {
        return Promise.resolve();
    }

    ledCharacteristic = await interactiveService.getCharacteristic(ledCharacteristicUUID);
    buttonCharacteristic = await interactiveService.getCharacteristic(buttonCharacteristicUUID);
    secureButtonCharacteristic = await interactiveService.getCharacteristic(secureButtonCharacteristicUUID);
    buttonCharacteristic.addEventListener("characteristicvaluechanged", handleButtonValueChanges);
    secureButtonCharacteristic.addEventListener("characteristicvaluechanged", handleSecureButtonValueChanges);
    document.querySelector("#on").disabled = false;
    document.querySelector("#securestart").disabled = false;
    document.querySelector("#start").disabled = false;
}

function handleButtonValueChanges(event) {
    handleValueChange("button", event.target.value.getUint8(0));
}

function handleSecureButtonValueChanges(event) {
    handleValueChange("secureButton", event.target.value.getUint8(0));
}

function handleValueChange(type, value) {
    var time = new Date();
    console.log(
        time.getHours() + ":" + time.getMinutes() + ":" + time.getSeconds() + " " + type + " value changed: " + value
    );
}

//---------------- LED Service ------------------------------

function toggleLED(value) {
    var dataView = new DataView(new ArrayBuffer(1));
    dataView.setUint8(0, value);
    return ledCharacteristic
        .writeValue(dataView)
        .then((_) => {
            console.log(value ? "LED set to on" : "LED set to off");
            document.querySelector("#on").disabled = value;
            document.querySelector("#off").disabled = !value;
        })
        .catch((error) => {
            console.log("[ERROR] On: " + error);
        });
}

//---------------- Notifications ------------------------------

function toggleNotification(value) {
    buttonCharacteristic[value ? "startNotifications" : "stopNotifications"]()
        .then((_) => {
            console.log(value ? "Start reading button changes" : "Stop reading button changes");
            document.querySelector("#start").disabled = value;
            document.querySelector("#stop").disabled = !value;
        })
        .catch((error) => {
            console.log("[ERROR] Start: " + error);
        });
}

function toggleSecureNotification(value) {
    secureButtonCharacteristic[value ? "startNotifications" : "stopNotifications"]()
        .then((_) => {
            console.log(value ? "Start reading secureButton changes" : "Stop reading secureButton changes");
            document.querySelector("#securestart").disabled = value;
            document.querySelector("#securestop").disabled = !value;
        })
        .catch((error) => {
            console.log("[ERROR] Start: " + error);
        });
}
