#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {

    std::set<int> duplicates;

    for (const int l_doc : search_server) {
        for (const int r_doc : search_server) {
            if (l_doc >= r_doc) {
                continue;
            }
            const std::map<std::string, double>& left = search_server.GetWordFrequencies(l_doc);
            const std::map<std::string, double>& right = search_server.GetWordFrequencies(r_doc);

            if (left.size() != right.size()) {
                continue;
            }

            std::map<std::string, double>::const_iterator l_it = left.begin();
            std::map<std::string, double>::const_iterator r_it = right.begin();

            bool equal = true;
            while (l_it != left.end()) {
                if (l_it->first != r_it->first) {
                    equal = false;
                    break;
                }
                l_it++;
                r_it++;
            }

            if (equal) {
                std::cout << "Found duplicate document id " << r_doc << "\n";
                duplicates.emplace(r_doc);
            }
        }
    }
    if (duplicates.size() > 0) {
        for (const int d : duplicates) {

            search_server.RemoveDocument(d);
        }
    }
}