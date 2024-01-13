#pragma once
#include <pgmspace.h>
const char htmlHomePage[] PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RFID Custom Config</title>
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #ecf0f1;
            /* Màu nền cho trang */
            letter-spacing: 1px;
        }

        .container {
            display: flex;
            width: 600px;
            /* Tổng chiều rộng của cả hai div */
            padding: 20px;
            border: 2px solid #3498db;
            background-color: #fff;
            text-align: center;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }

        .left {
            flex: 1;
            /* Chia chiều rộng tỉ lệ 1:1 */
            padding-right: 10px;
        }

        .right {
            flex: 1;
            /* Chia chiều rộng tỉ lệ 1:1 */
            padding-left: 10px;
            border-left: 1px solid #3498db;
            text-align: left;
            display: flex;
            flex-direction: column;
        }

        .top {
            margin-bottom: 10px;
            border-bottom: 1px solid #3498db;
            padding-bottom: 10px;
        }

        .line-divider {
            width: 100%;
            height: 1px;
            background-color: #3498db;
            margin: 10px 0;
        }

        .scroll-area {
            max-height: 250px;
            min-height: 250px;
            overflow-y: auto;
            border: 1px solid #3498db;
            padding: 10px;
            text-align: left;
            font-size: small;
        }

        .clear-button {
            margin-top: 10px;
            padding: 8px 16px;
            background-color: #3498db;
            color: #fff;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s ease-in-out;
        }

        .clear-button:hover {
            background-color: #2980b9;
        }

        /* Nút cho phần bên phải */
        .action-button {
            margin-top: 10px;
            padding: 8px 16px;
            background-color: #3498db;
            color: #fff;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s ease-in-out;
        }

        .action-button:hover {
            background-color: #2980b9;
        }

        /* Phần control cho phần bên phải */
        .control {
            text-align: center;
            margin-bottom: 15px;
        }

        .control-title {
            font-size: 18px;
            font-weight: bold;
        }

        /* Form cho phần bên phải */
        .form-container {
            margin-top: 0px;
            display: flex;
            flex-direction: column;
        }

        .form-input {
            margin-bottom: 10px;
            display: flex;
            flex-direction: column;
        }

        .form-input label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
        }

        .form-input input {
            width: calc(100% - 10px);
            padding: 8px;
            box-sizing: border-box;
        }
    </style>
</head>

<body>
    <div class="container">
        <div class="left">
            <div class="control">
                <div class="control-title">View</div>
                <!-- Nội dung control có thể được thêm vào đây -->
            </div>
            <div class="scroll-area">
                <!-- Nội dung có thể được thêm vào đây -->
                <!-- Dùng thẻ div với class "clear-button" để tạo nút "Clear" -->
            </div>
            <button class="clear-button" onclick="clearContent()">Clear</button>
        </div>
        <div class="right">
            <!-- Form cho phần bên phải -->
            <div class="control">
                <div class="control-title">Add License Plate</div>
                <!-- Nội dung control có thể được thêm vào đây -->
            </div>
            <div class="form-container">
                <div class="form-input">
                    <label for="name">Name:</label>
                    <input type="text" id="name" name="name" autocomplete="off" maxlength="16">
                </div>
                <div class="form-input">
                    <label for="license">License Plate:</label>
                    <input type="text" id="license" name="license" autocomplete="off" maxlength="16">
                </div>
                <div class="form-input">
                    <label for="cccd">CCCD:</label>
                    <input type="text" id="cccd" name="cccd" autocomplete="off" maxlength="16px">
                </div>
                <!-- Phần Authentication -->
                <div class="form-input">
                    <label for="authenticationKey">Authentication (KeyA):</label>
                    <input type="text" id="authenticationKey" name="authenticationKey" autocomplete="off" maxlength="17"
                        oninput="formatKey(this)" style="text-transform: uppercase;">
                </div>
                <button class="action-button" onclick="submitForm()">Submit</button>
            </div>
        </div>
    </div>

    <script>
        var ws;
        let name, license, cccd, authenticationKey;
        let view_area = document.querySelector('.scroll-area');
        const showArea = (content, status, show_status = true) => {
            const currentDate = new Date();
            const _time = `${currentDate.getHours()}:${currentDate.getMinutes()}:${currentDate.getSeconds()} ${currentDate.toDateString()}`;
            let _status;
            if (show_status) {
                _status = (status["err_code"] == 0) ? `<span style="color:green">SUCCESS</span>` : `<span style="color:red">FAILD: ${status["msg"]}</span>`;
            }
            let msg = `<div><b>${_time}</b><br>`;
            msg += content;
            if (!show_status) {
                msg += `<br>---<br>`;
            } else {
                msg += `<br>Status: ${_status}<br>---<br>`;
            }
            msg += "</div>";
            view_area.innerHTML += msg;
            view_area.scrollTop = view_area.scrollHeight;
        };
        function formatKey(input) {
            const cleanedInput = input.value.replace(/[^A-Za-z0-9]/g, '');
            let formatedKey = '';
            for (let i = 0; i < cleanedInput.length; i += 2) {
                formatedKey += cleanedInput.substr(i, 2) + '-';
            }
            formatedKey = formatedKey.slice(0, -1);
            input.value = formatedKey;
        }
        function clearContent() {
            view_area.innerHTML = '';
        }
        function submitForm() {
            // Xử lý khi nút "Submit" được nhấp
            name = document.getElementById('name').value;
            license = document.getElementById('license').value;
            cccd = document.getElementById('cccd').value;
            authenticationKey = document.getElementById('authenticationKey').value;
            if (name === '' || license === '' || cccd === '' || authenticationKey === '') {
                alert('Vui lòng điền đầy đủ thông tin!');
                return;
            }
            const msg = { type: "add_card", name, license, cccd, authenticationKey };
            ws.send(JSON.stringify(msg));
        }
        document.addEventListener("DOMContentLoaded", function () {
            if ("WebSocket" in window) {
                ws = new WebSocket('ws://' + window.location.host + ':81/');
                // ws = new WebSocket('ws://192.168.0.115:81/');
                ws.onopen = function () { };
                ws.onmessage = function (evt) {
                    const type = evt.data;
                    if (typeof type === 'string') {
                        const json = JSON.parse(evt.data);
                        console.log("Data Read:" + JSON.stringify(json));
                        const type = json["type"];
                        if (type == "card_info") {
                            const { card_uid, name, cccd, license_plate } = json;
                            const content = `<b>Read Card</b><br>Card UID: ${card_uid}<br>Name: ${name}<br>License: ${license_plate}<br>CCCD: ${cccd}`;
                            showArea(content, {}, false);
                        } else if (type == "add_status") {
                            const { error, reason } = json;
                            const content = `<b>Add Card</b><br>Name: ${name}<br>License: ${license}<br>CCCD: ${cccd}<br>Key: ${authenticationKey}`;
                            showArea(content, { "err_code": error, "msg": reason });
                        } else if (type == "other") {
                            showArea("<br>Đưa thẻ cần ghi, lại gần cảm biến<br>", {}, false);
                        }
                    }
                };
                ws.onclose = function () { };
            } else {
                alert("WebSocket NOT supported by your Browser!");
            }
        });
    </script>
</body>

</html>
)HTMLHOMEPAGE";
