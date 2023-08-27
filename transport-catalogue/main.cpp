#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"

int main() {
    json_reader::JSONReader reader(std::cin);
    reader.GenerateResponse(std::cout);
}
