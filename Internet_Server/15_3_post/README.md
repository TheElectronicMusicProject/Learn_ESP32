# How to use

    1) Connect the ESP32 to the router
    2) Connect the PC to the router
    3) Open Thunder Client
    4) Click on "New Request"
    5) Set a post request to `http://192.168.1.103/api/toggle-led`
    6) Add the header `Content-Type` with `application/json`
    7) Add one of the following body
        *   {
                "is_on":false
            }
        *   {
                "is_on":false
            }
    8) Click on "Send"
