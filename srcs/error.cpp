#include "webserv.hpp"

std::string ServerSocket::buildErrorFiles(const std::string error)
{
    std::string response = "<html><head><http-equiv=\"Content-type\" content=\"text/html;\
    charset=utf-8\"></head><body></body></html><html><head><title>" + error + "</title></head>\
    <body><center><h1>" + error + "</h1></center><hr><center>webserv</center></body></html>";

    return ("HTTP/1.1 " + error + "\r\n\r\n" + response);
}

std::string ServerSocket::callErrorFiles(const int error)
{
    std::map<std::string, std::string>::iterator it = currentServ.getServError(std::to_string(error));
    if (it != currentServ.getErrorEnd())
        return ("HTTP/1.1 302 Found\r\nLocation: " + it->second + "\r\n\r\n");
    switch(error)
    {
        case 400:
            return (buildErrorFiles("400 Bad Request"));
        case 401:
            return (buildErrorFiles("401 Unauthorized"));
        case 402:
            return (buildErrorFiles("402 Payment Required"));
        case 403:
            return (buildErrorFiles("403 Forbidden"));
        case 404:
            return (buildErrorFiles("404 Not Found"));
        case 405:
            return (buildErrorFiles("405 Method Not Allowed"));
        case 406:
            return (buildErrorFiles("406 Not Acceptable"));
        case 407:
            return (buildErrorFiles("407 Proxy Authentication Required"));
        case 408:
            return (buildErrorFiles("408 Request Timeout"));
        case 409:
            return (buildErrorFiles("409 Conflict"));
        case 410:
            return (buildErrorFiles("410 Gone"));
        case 411:
            return (buildErrorFiles("411 Length Required"));
        case 412:
            return (buildErrorFiles("412 Precondition Failed"));
        case 413:
            return (buildErrorFiles("413 Payload Too Large"));
        case 414:
            return (buildErrorFiles("414 URI Too Long"));
        case 415:
            return (buildErrorFiles("415 Unsupported Media Type"));
        case 416:
            return (buildErrorFiles("416 Range Not Satisfiable"));
        case 417:
            return (buildErrorFiles("417 Expectation Failed"));
        case 418:
            return (buildErrorFiles("418 I'm a teapot"));
        case 421:
            return (buildErrorFiles("421 Misdirected Request"));
        case 422:
            return (buildErrorFiles("422 Unprocessable Content"));
        case 423:
            return (buildErrorFiles("423 Locked"));
        case 424:
            return (buildErrorFiles("424 Failed Dependency"));
        case 425:
            return (buildErrorFiles("425 Too Early"));
        case 426:
            return (buildErrorFiles("426 Upgrade Required"));
        case 428:
            return (buildErrorFiles("428 Precondition Required"));
        case 429:
            return (buildErrorFiles("429 Too Many Requests"));
        case 431:
            return (buildErrorFiles("431 Request Header Fields Too Large"));
        case 451:
            return (buildErrorFiles("451 Unavailable For Legal Reasons"));
        case 500:
            return (buildErrorFiles("500 Internal Server Error"));
        case 501:
            return (buildErrorFiles("501 Not Implemented"));
        case 502:
            return (buildErrorFiles("502 Bad Gateway"));
        case 503:
            return (buildErrorFiles("503 Service Unavailable"));
        case 504:
            return (buildErrorFiles("504 Gateway Timeout"));
        case 505:
            return (buildErrorFiles("505 HTTP Version Not Supported"));
        case 506:
            return (buildErrorFiles("506 Variant Also Negotiates"));
        case 507:
            return (buildErrorFiles("507 Insufficient Storage"));
        case 508:
            return (buildErrorFiles("508 Loop Detected"));
        case 510:
            return (buildErrorFiles("510 Not Extended"));
        case 511:
            return (buildErrorFiles("511 Network Authentication Required"));
        default:
        {
            std::cerr << "unsupported error" << std::endl;
            checkFdSets();
		    return (callErrorFiles(500));
        }

    }
    return (NULL);
}