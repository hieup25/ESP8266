const textTemp      = document.querySelector('#temp');
const textHumi      = document.querySelector('#humi');
const colorPicker   = document.querySelector('#color');
const applyBtn      = document.querySelector('#apply-btn');

applyBtn.addEventListener('click', async () => {
    await fetch(`http://myserver.local/set-color?value=${colorPicker.value.substring(1)}`);
    // GET http://myserver.local/set-color?value=ff0000
});

// Mỗi 2 giây request xuống ESP8266 lấy thông tin từ DHT11 1 lần
setInterval(async () => {
    // const response = await fetch(`http://myserver.local/get-dht11`);
    // const jsonData = await response.json();
    const jsonData = {"t":23.0, "h":90.0}
    // jsonData có format {"t":value, "h":value}
    textTemp.innerText = jsonData.t + "°C"; // gán nhiệt độ vào span có id là temp
    textHumi.innerText = jsonData.h + "%"; // gán độ ẩm vào span có id là humi
}, 2000);