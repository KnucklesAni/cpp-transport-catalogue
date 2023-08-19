#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
  transport_catalogue::TransportCatalogue catalogue;

  size_t N;
  std::cin >> N;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (size_t line_no = 0; line_no < N; ++line_no) {
    std::string linе;
    std::getline(std::cin, linе);
    std::string_view line = linе;
    if (line.substr(0, 4) == "Bus ") {
      line = line.substr(4);
      catalogue.Add(input_reader::ReadBusInfo(catalogue, line));
    } else if (line.substr(0, 5) == "Stop ") {
      line = line.substr(5);
      const transport_catalogue::Stop *stop_from =
          catalogue.Add(input_reader::ReadStopInfo(line));
      input_reader::ReadStopToStopInfo(stop_from, catalogue, line);
    } else {
      throw std::logic_error{"Line must start with Bus or Stop!\nLine" +
                             std::to_string(line_no + 1)};
    }
  }

  size_t Q;
  std::cin >> Q;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (size_t line_no = 0; line_no < Q; ++line_no) {
    std::string linе;
    std::getline(std::cin, linе);
    std::string_view line = linе;
    if (line.substr(0, 4) == "Bus ") {
      line = line.substr(4);
      stat_reader::ProvideBusInfo(std::cout, catalogue, line);
    } else if (line.substr(0, 5) == "Stop ") {
      line = line.substr(5);
      stat_reader::ProvideStopInfo(std::cout, catalogue, line);
    } else {
      throw std::logic_error{"Line must start with Bus!\nLine" +
                             std::to_string(line_no + N + 2)};
    }
  }
}
