const tamano = 200;
const video = document.getElementById("video");
let model = null;
let maxPredictions = 0;
let c = 0;

(async () => {
    try {
        print("Cargando modelo IA...");
        model = await tmImage.load("model.json", "metadata.json");
        maxPredictions = model.getTotalClasses();
        print("Modelo cargado");
    } catch (err) {
        print("Error al cargar el modelo:", err);
    }
})();

window.onload = () => {
    ["ip_camara", "ip_control"].forEach(id => {
        document.getElementById(id).addEventListener("keypress", (e) => {
            if (e.key === "Enter") {
                print(id === "ip_camara" ? "Cargando cámara ..." : "Robot conectado");
                id === "ip_camara" ? cargar_camara() : cargar_control();
            }
        });
    });
};

function cargar_camara() {
    const ip_camara = document.getElementById("ip_camara").value.trim();
    if (ip_camara) {
        video.crossOrigin = "anonymous";
        video.src = `http://${ip_camara}:8080/video`;
        video.onload = () => {
            print("La cámara se ha cargado correctamente");
            setTimeout(procesarCamara, 1000);
        };
        video.onerror = () => print("Error al cargar la cámara. Verifica la URL.");
    }
}

function cargar_control() {
    ip_control = document.getElementById("ip_control").value.trim();
}

function procesarCamara() {
    if (video.complete && video.naturalWidth !== 0) {
        predecir();
    }
    requestAnimationFrame(procesarCamara);
}

async function predecir() {
    if (model) {
        const predictions = await model.predict(video);
        const bestPrediction = predictions.reduce((max, pred) => pred.probability > max.probability ? pred : max);
        document.getElementById("label-container").innerHTML = `${bestPrediction.className}, con  ${(100 * bestPrediction.probability).toFixed(2)}% de probabilidad`;
    }
}

document.addEventListener("keydown", function(event) {
    // Evitar procesar la misma tecla si se mantiene presionada
    if (event.repeat) return;

    const key = event.key.toUpperCase();

    const button = document.getElementById(`btn${key}`);
    if (button) {
        button.style.backgroundColor = "#0e5979";
        button.click();
        setTimeout(() => button.style.backgroundColor = "#343a40", 200);
    }
    if (c == 2){c = 0;};
    if (["W", "A", "S", "D"].includes(key) && c == 0) {
        desplazar(key);
    }
});

function desplazar(direccion) {
    c=+ 1;
    fetch(`http://${ip_control}/${direccion}`)
        .then(response => response.json())
        .then(data => print(`Mensaje del robot: ${data.estado}`))
        .catch(error => print(`Error: ${error}`));
}

function print(message) {
    const consoleOutput = document.getElementById("console-output");
    const newMessage = document.createElement("div");
    newMessage.textContent = `>> ${message}`;
    consoleOutput.appendChild(newMessage);
    consoleOutput.scrollTop = consoleOutput.scrollHeight;
    // newMessage.scrollIntoView({ behavior: "smooth", block: "end" });
}

