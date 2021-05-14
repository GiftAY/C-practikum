#include "search_server.h"

#include <iomanip>

template <typename Func>
void RunTestImpl(Func f, const std::string& s) {
    f();
    std::cerr << s << " OK" << std::endl;
}

void AssertImpl(bool expression,
    const std::string& str,
    const std::string& file,
    const std::string& function,
    const unsigned line,
    const std::string& hint) {

    if (!expression) {
        std::cout << file << "(" << line << "): " << function << ": ASSERT(" << str << ") failed. ";
        if (!hint.empty()) {
            std::cout << "Hint: " << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "(" << line << "): " << func << ": ";
        std::cerr << "ASSERT_EQUAL(" << t_str << ", " << u_str << ") failed: ";
        std::cerr << t << " != " << u << ".";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}


void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city";
    const std::vector<int> ratings = { 1, 2, 3 };
    
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in");
        ASSERT_EQUAL_HINT(found_docs.size(), 1, " 1 document found");
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

   
    {
        SearchServer server;
        server.SetStopWords("in the");
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in").empty(), " empty");
    }
}

void AddingDocuments() {
    const int doc_id1 = 0;
    const int doc_id2 = 1;
    const int doc_id3 = 2;
    const int doc_id4 = 3;

    const std::string content1 = "1word1 1word2 1word3 1word4";
    const std::string content2 = "2word1 2word2 2word3 2word4";
    const std::string content3 = "3word1 3word2 3word3 3word4 3word3 3word4";
    const std::string content4 = "4word1 4word2 4word3 4word4";

    const std::vector<int> ratings = { 1, 2, 3 };
    
    {
        SearchServer server;
        const std::vector<Document>& found_documents = server.FindTopDocuments("word1");
        ASSERT(found_documents.empty());
    }

    {
        SearchServer server;
        server.SetStopWords("stop1 stop2 stop3");
        const std::vector<Document>& fd = server.FindTopDocuments("word2");
        const std::vector<Document>& fds = server.FindTopDocuments("word2", DocumentStatus::BANNED);
        const std::vector<Document>& fdsp = server.FindTopDocuments("word2", [](const int id, const DocumentStatus ds, int rating) {return rating > 1; });
        ASSERT(fd.empty());
        ASSERT(fds.empty());
        ASSERT_HINT(fdsp.empty(), "vector must be empty");
        ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0, "Documents count == 0");
    }


    {
        SearchServer server;
        server.SetStopWords("1word1 2word2 2word3");
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, { 1, 2, 3 });
        server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, { 4, 5, 6, 7, 8 });
        server.AddDocument(doc_id3, content3, DocumentStatus::IRRELEVANT, { 1, 3, 4, 5, 6, 7, 8 });
        server.AddDocument(doc_id4, content4, DocumentStatus::REMOVED, { 4, 5, 6, 7, 8, 20, 9 });

        {
            const std::vector<Document>& fd = server.FindTopDocuments("1word2");
            ASSERT_EQUAL(fd[0].id, doc_id1);
        }

   
        {
            const std::vector<Document>& fd = server.FindTopDocuments("2word2");
            ASSERT_HINT(fd.empty(), "vector must be empty");
        }

       
        {
            const std::vector<Document>& fd1 = server.FindTopDocuments("1word2 -1word3");
            ASSERT(fd1.empty());
            const std::vector<Document>& fd2 = server.FindTopDocuments("2word1 -1word3", DocumentStatus::BANNED);
            ASSERT(fd2[0].id == doc_id2);
        }

       
        {
            const auto& [w1, ds1] = server.MatchDocument("1word1 1word2 2word1 2word2", doc_id1);
            const auto& [w2, ds2] = server.MatchDocument("1word1 1word2 2word1 2word2", doc_id2);
            const auto& [w3, ds3] = server.MatchDocument("1word1 1word2 -2word1 2word2", doc_id1);
            const auto& [w4, ds4] = server.MatchDocument("-1word2 -2word1 2word2", doc_id1);
            ASSERT(w1[0] == "1word2"); ASSERT(ds1 == DocumentStatus::ACTUAL);
            ASSERT(w2[0] == "2word1"); ASSERT(ds2 == DocumentStatus::BANNED);
            ASSERT(w3[0] == "1word2"); ASSERT(ds3 == DocumentStatus::ACTUAL);
            ASSERT(w4.empty());
        }

    }


    {
        SearchServer server;
        server.SetStopWords("1word1 2word2 2word3");
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, { 1, 2, 3 });
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, { 4, 5, 6, 7, 8 });
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, { 1, 3, 4, 5, 6, 7, 8 });
        server.AddDocument(doc_id4, content4, DocumentStatus::ACTUAL, { 4, 5, 6, 7, 8, 20, 9 });

        const std::vector<Document>& fd = server.FindTopDocuments("1word2 -2word1 3word1 3word2 3word3 4word1");
        ASSERT(fd[1].relevance < fd[0].relevance); ASSERT(fd[0].rating < fd[2].rating);
        ASSERT(fd[2].relevance < fd[1].relevance); ASSERT(fd[1].rating < fd[0].rating);
        ASSERT(fd[0].relevance == 0.92419624074659368); ASSERT(fd[0].rating == 4);

    }
}




void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(AddingDocuments);
}



int main() {
    TestSearchServer();
    std::cout << "Search server testing finished" << std::endl;
}