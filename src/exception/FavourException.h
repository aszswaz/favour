//
// Created by aszswaz on 2022/1/21.
//

#ifndef FAVOUR_FAVOUREXCEPTION_H
#define FAVOUR_FAVOUREXCEPTION_H

#include <exception>

namespace favour {
    class FavourException : public std::exception {
    private:
        const char *message;
    public:
        FavourException(const char *message) {
            this->message = message;
        }

        const char *what() {
            return this->message;
        }
    };
}

#endif //FAVOUR_FAVOUREXCEPTION_H
