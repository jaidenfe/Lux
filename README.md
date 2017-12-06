# Lux

This project aims to create a low cost home automation illumination device that will turn any lamp or light source into a smart light source. Instead of replacing the light blub or wall socket, we aim to replace the light socket itself, allowing the user to conintue using the light blubs they already have for their homes. 

<h3>Getting Started</h3>

The project has two devices: the Lux Hub and the Lux SmartSocket.

The Lux Hub is run on a raspberry pi. To set up the Lux Hub download the hub_setup script in the top-most directory. Move this to the raspberry pi. Make sure the raspberry pi has internet access and run the script by typing './hub_setup'. You may have to run this as root. The Lux Hub is now setup and has the most up to date software from this Lux repository.

The Lux Hub will now automatically run the Hub software when it is powered up. The user must plug the raspberry pi into their router and power the raspberry pi on for the system to work. The user can now access the Hub Dashboard by visiting 192.168.1.XXX, where XXX is a number from 2 - 254. We are looking into ways to set a domain name instead of an IP address without requiring the user to modify their current network setup.

The Lux SmartSocket is a custom peice of hardware with custom firmware (located in 'Lux/src/firmware/ESP8266_LUX_FIRMWARE_V1/'). If you are interested in aquiring a device please reach out to our development team.

<h3>Contributor</h3>
Xuanyu Dong (Front End)
