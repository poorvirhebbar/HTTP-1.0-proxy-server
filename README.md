# HTTP-1.0-proxy-server
Name : Poorvi R Hebbar

ID : 20196053

Assignment #2

Help : used stackoverflow and the guides for given in the reference for many chunks of my code

Time : 3 days

Assessment : It helped me to get a clear understanding on how proxies work, though i had wondered about it a lot before.

Description :

	This HTTP proxy server made in C++ language. It supports only HTTP/1.0 protocol GET method requests. For the other requests, try using telnet directly for giving the exact error. 

How to Run :

	$make all
	$./proxy <port_number> (if there are no blacklisted web pages)
	$./proxy <port_number> < <file containing the blacklisted pages with a \n between every 2 page>

	proxy server starts running on ur local machine port 6789 . 
	Port number can beany number below 65k other than first 1000 system registered ports

Possible errors or bugs:

	For non,HTTP1.0 GET requests, kindly use the telnet directly.
	Give sometime before using the same port on the same host.
	Error-checking is done at every step . Errors will be displayed in terminal at which the proxy is running.

