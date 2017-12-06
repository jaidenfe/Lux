/**
 * Created by Xuanyu on 11/8/2017.
 */

// setInterval("loading_page()", 5000);
var request_for_page = {"cmd": "2"};

var deviceState = {};
function loading_page() {
    sendRequest(true, request_for_page, "/status_req",0);
}

function sendRequest(upd, data, type,check) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", type, true);
    xhttp.setRequestHeader("Content-type", "application/json");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (upd) {
              if(check == 0){
                onPageLoad(this);
              }
              else{
                var resp = xhttp.responseText;
                myDevices = JSON.parse(resp);
                var groupStatus = {};
                for(x in myDevices){
                  if(!(myDevices[x].data.group_name in groupStatus)){
                    groupStatus[myDevices[x].data.group_name] = true;
                  }
                  if(myDevices[x].data.level == 10){
                    document.getElementById(myDevices[x].serial + "state").checked = true;
                    deviceState[myDevices[x].serial] = true;
                    var devicePictureState = checkOnOrOffState(deviceState[myDevices[x].serial]);
                    document.getElementById(myDevices[x].serial).getElementsByClassName("Device_img")[0].src = devicePictureState.src;
                  }
                  else{
                    document.getElementById(myDevices[x].serial + "state").checked = false;
                    deviceState[myDevices[x].serial] = false;
                    var devicePictureState = checkOnOrOffState(deviceState[myDevices[x].serial]);
                    document.getElementById(myDevices[x].serial).getElementsByClassName("Device_img")[0].src = devicePictureState.src;
                    groupStatus[myDevices[x].data.group_name] = false;
                  }
                }
                for(x in groupStatus){
                  document.getElementById(x+"state").checked = groupStatus[x];
                }
              }
            }
            else {
                var resp = xhttp.responseText;
                myDevices = JSON.parse(resp);
                if (myDevices.data.level == 10) {
                    deviceState[myDevices.serial] = true;
                    document.getElementById(myDevices.serial + "state").checked = true;
                }
                else {
                    deviceState[myDevices.serial] = false;
                    document.getElementById(myDevices.serial + "state").checked = false;
                    document.getElementById(myDevices.data.group_name + "state").checked = false;
                }
                var valueG = true;
                var groupD = document.getElementById(myDevices.data.group_name).getElementsByTagName("ul")[0].getElementsByTagName("li");
                for (var i = 0; i < groupD.length; i++) {
                    var deviceID = groupD[i].id+"state";
                    // alert(deviceID);
                    if(document.getElementById(deviceID).checked == false){
                      valueG = false;
                      break;
                    }
                }
                document.getElementById(myDevices.data.group_name + "state").checked = valueG;
                var devicePictureState = checkOnOrOffState(deviceState[myDevices.serial]);

                // alert(myDevices.serial);
                document.getElementById(myDevices.serial).getElementsByClassName("Device_img")[0].src = devicePictureState.src;
                // alert(devicePictureState);
            }
        }
    }
    var param = JSON.stringify(data);
    xhttp.send(param);
}

function onPageLoad(xhttp) {
    var resp = xhttp.responseText;
    myDevices = JSON.parse(resp);
    var devices = document.getElementById("user_devices");
    while (devices.hasChildNodes()) {
        devices.removeChild(devices.lastChild);
    }
    var mylistG = document.createElement("ul");
    mylistG.id = "myLuxGroup";
    mylistG.className = "list-group";

    document.getElementById("user_devices").appendChild(mylistG);
    for (x in myDevices) {
        var deviceGID = myDevices[x].data.group_name;
        if (document.getElementById(deviceGID) === null) {
            createNewGroup(mylistG, deviceGID);
        }
        var groupList = document.getElementById(deviceGID);
        var deviceID = myDevices[x].serial;
        if (myDevices[x].data.level == 10) {
            deviceState[myDevices[x].serial] = true;
        }
        else {
            deviceState[myDevices[x].serial] = false;
        }
        var device_img = checkOnOrOffState(deviceState[deviceID]);
        var device = document.createElement("li");
        var deviceInfo = document.createElement("div");
        var deviceName = document.createElement("h2");
        var deviceSerial = document.createElement("h4");
        var sliderCon = document.createElement("div");
        var stateSlider = document.createElement('label');
        var checkBox = document.createElement('input');
        var handler = document.createElement('span');
        sliderCon.className = "slider-con my-auto";
        deviceInfo.className = "my-auto deviceName";
        deviceName.className = "";
        deviceSerial.className = "text-muted text-center";
        checkBox.type = "checkbox";
        checkBox.className = "";
        checkBox.id = myDevices[x].serial + "state";
        handler.innerHTML = "ON OFF";
        handler.className = "slider round";
        stateSlider.className = "switch";
        deviceInfo.appendChild(deviceName);
        deviceInfo.appendChild(deviceSerial);
        stateSlider.appendChild(checkBox);
        stateSlider.appendChild(handler);
        sliderCon.appendChild(stateSlider);
        if (myDevices[x].data.level == 10) {
            checkBox.checked = true;
        } else {
            checkBox.checked = false;
            document.getElementById(deviceGID + "state").checked = false;
        }
        deviceName.innerHTML = myDevices[x].data.name;
        deviceSerial.innerHTML = "- Serial number: " + myDevices[x].serial + " -";
        deviceName.style.textAlign = "center";
        device.className = "list-group-item device";
        device.style.display = "flex";
        device.id = deviceID;
        device.appendChild(deviceInfo);
        device.appendChild(sliderCon);
        device.appendChild(device_img);
        handler.addEventListener("click", onState(myDevices[x].data.name, deviceID, deviceGID, 0));
        groupList.getElementsByTagName("ul")[0].appendChild(device);
    }

    document.getElementById("user_devices").appendChild(mylistG);
}

function checkOnOrOffState(deviceState) {
    if (deviceState == true) {
        var image = document.createElement('img');
        image.src = "static/images/lighton.jpg";
        image.className = "Device_img";
        return image;
    }
    else if (deviceState == false) {
        var image = document.createElement('img');
        image.src = "static/images/lightoff.jpg";
        image.className = "Device_img";
        return image;
    }
}
function onState(deviceName, deviceID, deviceGroup, group) {
    return function () {
        //deviceState[deviceID]==0
        // alert(value);
        var state = document.getElementById(deviceID + "state").checked;
        var groupstate = document.getElementById(deviceGroup + "state").checked;
        if (group) {
            if (!groupstate) {
                var data = {
                    "cmd": "4",
                    "uuid": "0",
                    "serial": deviceID,
                    "data": {"group_name": deviceGroup, "name": deviceName, "level": 10}
                };
            } else if (groupstate) {
                var data = {
                    "cmd": "4",
                    "uuid": "0",
                    "serial": deviceID,
                    "data": {"group_name": deviceGroup, "name": deviceName, "level": 0}
                };
            }
        } else {
            if (!state) {
                var data = {
                    "cmd": "4",
                    "uuid": "0",
                    "serial": deviceID,
                    "data": {"group_name": deviceGroup, "name": deviceName, "level": 10}
                };
            } else if (state) {
                var data = {
                    "cmd": "4",
                    "uuid": "0",
                    "serial": deviceID,
                    "data": {"group_name": deviceGroup, "name": deviceName, "level": 0}
                };
            }
        }
        sendRequest(false, data, "/update_req",0);
    }
}
function createNewGroup(mylistG, deviceGID) {
    var groupList = document.createElement("li");
    var groupInfo = document.createElement("div");
    var groupName = document.createElement("a");

    var sliderCon = document.createElement("div");
    var stateSlider = document.createElement('label');
    var checkBox = document.createElement('input');
    var handler = document.createElement('span');
    sliderCon.className = "slider-con my-auto";
    sliderCon.style = "position:relative;float:right";
    checkBox.type = "checkbox";
    checkBox.className = "";
    checkBox.id = myDevices[x].data.group_name + "state";
    checkBox.checked = true;
    handler.innerHTML = "ON OFF";
    handler.className = "slider round";
    stateSlider.className = "switch";
    stateSlider.appendChild(checkBox);
    stateSlider.appendChild(handler);
    sliderCon.appendChild(stateSlider);
    groupList.appendChild(sliderCon);
    groupList.onClick = "return true";
    handler.addEventListener("click", groupState(deviceGID));
    groupInfo.className = "my-auto deviceGroup";
    groupName.setAttribute("data-toggle", "collapse");
    groupName.setAttribute("href", "#child" + deviceGID);
    groupName.setAttribute("data-target", "#child" + deviceGID);
    groupName.addEventListener("click", function () {
            if (document.getElementsByClassName("groupName"+deviceGID)[0].getAttribute("aria-expanded")=="false") {
                groupName.innerHTML = "Group: " + deviceGID + "  " + "<i class='fa fa-sort-asc' aria-hidden='true'></i>";
            }
            else {
                groupName.innerHTML = "Group: " + deviceGID + "  " + "<i class='fa fa-sort-desc' aria-hidden='true'></i>";
            }
        }
    );
    groupName.className = "groupName groupName"+deviceGID;
    groupList.className = "list-group-item group";
    groupList.id = deviceGID;
    groupInfo.appendChild(groupName);
    var mylistD = document.createElement("ul");
    mylistD.id = "child" + deviceGID;
    mylistD.className = "list-device collapse";
    groupInfo.appendChild(mylistD);
    groupName.innerHTML = "Group: " + deviceGID + "  ";

    groupName.innerHTML += "<i class='fa fa-sort-desc' aria-hidden='true'></i>";
    groupList.appendChild(groupInfo);
    mylistG.appendChild(groupList);
}

function groupState(deviceGroup) {
    return function () {
        var groupD = document.getElementById(deviceGroup).getElementsByTagName("ul")[0].getElementsByTagName("li");
        // alert(groupList.getElementsByTagName("ul")[0].getElementsByTagName("li")[0].id);
        var value = document.getElementById(deviceGroup).getElementsByTagName("div")[0].getElementsByTagName("label")[0].getElementsByTagName("input")[0].checked;
        for (var i = 0; i < groupD.length; i++) {
            var deviceID = groupD[i].id;
            var deviceName = groupD[i].getElementsByTagName("div")[0].getElementsByTagName("h2")[0].innerHTML;
            onState(deviceName, deviceID, deviceGroup, 1)();
        }

    }
}
function updateState(){
  sendRequest(true, request_for_page, "/status_req",1);
}
setInterval("updateState()", 5000);
