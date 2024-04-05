# ESP32-Cam for monitoring 3D Prints


## How to Use

1. Open this sketch in the `Arduino IDE` and compile it
2. Modify the `WIFI_SSID`, `WIFI_PASS`, `API_KEY`, and `PRINTER_ID` variables to match your settings.
3. Select your board and Port, then upload the sketch
4. View your print and change your settings by visiting the Web App at: [https://app.printpal.io](https://app.printpal.io)

## Notes

- Notifications and Pausing are currently not implemented. Notifications will be implemented in the next build
- Some ESP32 boards have a timeout issue when sending requests, preventing them from communicating with the server when the images are too large.

## Where to buy
You can buy an ESP32 cam board from amazon, we reccomend this one: [ESP32-cam kit](https://amzn.to/4cWzBwS)

## API Key

Get your API key by signing up for a free account at: [app.printpal.io](https://app.rpintpal.io).

Get your Premium API key by signing up at: [printpal.io](https://printpal.io/printwatch/)

## Docs

Read more about setup and usage here: [docs](https://docs.printpal.io)
