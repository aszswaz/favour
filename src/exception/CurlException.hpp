//
// Created by aszswaz on 2022/1/17.
//

#ifndef FAVOUR_CURLEXCEPTION_H
#define FAVOUR_CURLEXCEPTION_H

#include <exception>

class CurlException : public std::exception {
private:
    const char *message;

public:
    explicit CurlException(const char *message) : message(message) {
        this->message = message;
    }

    const char *what() {
        return this->message;
    }
};


#endif //FAVOUR_CURLEXCEPTION_H
