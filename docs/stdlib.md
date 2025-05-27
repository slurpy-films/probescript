# Probescript Standard Library
## This file contains the documentation of probescript's standard library.

## http
### This module is used to create a http server and to send http requests. Members:

### http.get(url: string, req: map)
This function is used to send a GET request. The url is a string, and req is a map of body, a string, and headers, a map. It automatically applies headers like Content-Length and Connection. It returns an object with these properties:
- body() - A function that will return the response body, as a string
- status - A number containing the status of the request

### http.post(url: string, req: map)
The same as http.get, but sends a POST request.

### http.Server()
This class provides functionality to create a http server. When you instance it with the **new** keyword, you get an object with these methods:
- Server.get(path: string, handler: function) - Set the get handler for a path. The handler will be called with two arguments: req and res. "req" is an object containing the path, method, headers, cookies, and a function raw() that will return the raw request as a string. "res" is an object containing functions used for responding to the request. It has these properties: send, used for sending raw text, html, used for responding with html, cookie, used for setting a cookie like this: res.cookie("name", "value"), contentType for setting the content type that will be sent by the send function. 
- Server.post(), Server.put() ... - Same as get but different methods.

## json
### This module is used for working with the JSON format. Members:

### json.parse(text: string)
This function is used for turning text in the JSON format into an object that is usable in probescript. 

### json.stringify(obj: object)
This function is used for turning a probescript object into valid json.

## random
### This module is used for creating random numbers. Members:

### random.randInt(min: number, max: number)
This function returns a random integer between min and max.

### random.rand()
This function returns a random number between 0 and 1.

## date
### This module is used for working with time and dates. Members:

### date.stamp(format: string)
This function returns a timestamp in the format given by the **format** parameter. Possible formats:
- "milli" returns a unix timestamp in milliseconds (milliseconds since the first of january, 1970)
- "sec" returns the same but in seconds
- "min" returns the same but in minutes
- "hour" returns the same but in hours
