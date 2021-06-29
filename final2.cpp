#include "search_server.h"

#include <iomanip>

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__,  hint)
#define ASSERT(expr)  AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__,  " ")

#define ASSERT_EQUAL_HINT(left, right, hint) AssertEqualImpl((left), (right), #left, #right, __FILE__, __FUNCTION__, __LINE__, hint)
#define ASSERT_EQUAL(left, right) ASSERT_EQUAL_HINT(left, right, " ");

#define RUN_TEST(func)  RunTestImpl((func), #func)

template <typename Func>
void RunTestImpl(Func f, const std::string& s) {
    f();
    std::cerr << s << "OK" << std::endl;
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



SearchServer GetSearchServer() {
    const int doc_id1 = 0;
    const int doc_id2 = 1;
    const int doc_id3 = 2;
    const int doc_id4 = 3;
    const int doc_id5 = 4;
    const int doc_id6 = 5;

    const std::string content1 = "1word1 1word2 1word3 1word4";
    const std::string content2 = "2word1 2word2 2word3 2word4";
    const std::string content3 = "3word1 3word2 3word3 3word4 3word3 3word4";
    const std::string content4 = "4word1 4word2 4word3 4word4";
    const std::string content5 = "5word1 5word2 5word3 5word4 5word3 5word4";
    const std::string content6 = "6word1 6word2 6word1 6word2";

    SearchServer server;
    server.SetStopWords("1word1 2word2 3word3");
    server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, { 4, 5, 6, 7, 8 });
    server.AddDocument(doc_id3, content3, DocumentStatus::IRRELEVANT, { 1, 3, 4, 5, 6, 7, 8 });
    server.AddDocument(doc_id4, content4, DocumentStatus::REMOVED, { 4, 5, 6, 7, 8, 20, 9 });
    server.AddDocument(doc_id5, content5, DocumentStatus::ACTUAL, { 5, 1, 3, 4, 5, 6, 7, 8 });
    server.AddDocument(doc_id6, content6, DocumentStatus::ACTUAL, { 9, 4, 5, 6, 7, 8, 20, 9 });

    return server;

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

void TAddingDocuments() {
    SearchServer server = GetSearchServer();

    const std::vector<Document>& fd0 = server.FindTopDocuments("word1");
    ASSERT_HINT(fd0.empty(), "this word doesn't exist");

    const std::vector<Document>& fd1 = server.FindTopDocuments("1word2");
    const std::vector<Document>& fd2 = server.FindTopDocuments("2word2", DocumentStatus::BANNED);
    const std::vector<Document>& fd3 = server.FindTopDocuments("1word2");

    ASSERT(fd1.size() == 1);
    ASSERT(fd2.empty());       
    ASSERT_EQUAL(fd3[0].id, 0);
    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 6, "Documents count == 6");
}


void TStopWords(){
    SearchServer server = GetSearchServer();

    const std::vector<Document>& fd = server.FindTopDocuments("2word2");
    ASSERT_HINT(fd.empty(), " this vector must be empty");

}

void TMinusWords() {
    SearchServer server = GetSearchServer();

    const std::vector<Document>& fd1 = server.FindTopDocuments("1word2 -1word3");
    ASSERT(fd1.empty());
    const std::vector<Document>& fd2 = server.FindTopDocuments("2word1 -1word3", DocumentStatus::BANNED);
    ASSERT_EQUAL(fd2[0].id, 1);

}
void TMatchingDocuments() {
    SearchServer server = GetSearchServer();

    const auto& [w1, ds1] = server.MatchDocument("1word1 1word2 2word1 2word2", 0);
    const auto& [w2, ds2] = server.MatchDocument("1word1 1word2 2word1 2word2", 1);
    const auto& [w3, ds3] = server.MatchDocument("1word1 1word2 -2word1 2word2", 0);
    const auto& [w4, ds4] = server.MatchDocument("-1word2 -2word1 2word2", 0);

    ASSERT(w1[0] == "1word2"); ASSERT(ds1 == DocumentStatus::ACTUAL);
    ASSERT(w2[0] == "2word1"); ASSERT(ds2 == DocumentStatus::BANNED);
    ASSERT(w3[0] == "1word2"); ASSERT(ds3 == DocumentStatus::ACTUAL);
    ASSERT(w4.empty());

}

void TestSortByRelevance() {
    SearchServer server = GetSearchServer();
    const std::string query = "1word2 2word1 3word1 3word2 3word3 4word1 5word5 5word2 6word3 6word1";
    const std::vector<Document>& fd = server.FindTopDocuments(query);
    ASSERT(fd[1].relevance < fd[0].relevance);
    ASSERT(fd[2].relevance < fd[1].relevance);
    ASSERT(fd[1].relevance < fd[0].relevance);      
    ASSERT(fd[2].relevance < fd[1].relevance);      
}
void TestOfRating() {
    SearchServer server = GetSearchServer();
    const std::string query = "-1word2 -2word1 3word1 3word2 3word3 4word1 5word5 5word2 6word3 6word1";
    const std::vector<Document>& fd = server.FindTopDocuments(query);

    ASSERT_EQUAL(fd[0].rating, 8);
    ASSERT_HINT(fd[1].rating < fd[0].rating, "");
}
void TSortingByPredicate() {
    SearchServer server = GetSearchServer();
    const std::string query = "-1word2 -2word1 3word1 3word2 3word3 4word1 5word5 5word2 6word3 6word1";

    
    const std::vector<Document>& fdsp = server.FindTopDocuments(query, [](const int id, const DocumentStatus ds, int rating) {return rating < 8 && 2 < rating; });

    ASSERT_EQUAL_HINT(fdsp.size(), 2, "docs with rating 4");
}

void TestByStatus() {
    SearchServer server = GetSearchServer();
    const std::string query = "-1word2 -2word1 3word1 3word2 3word3 4word1 5word5 5word2 6word3 6word1";
    const std::vector<Document>& fdA = server.FindTopDocuments(query, DocumentStatus::ACTUAL);
    const std::vector<Document>& fdB = server.FindTopDocuments(query, DocumentStatus::BANNED);
    const std::vector<Document>& fdI = server.FindTopDocuments(query, DocumentStatus::IRRELEVANT);
    const std::vector<Document>& fdR = server.FindTopDocuments(query, DocumentStatus::REMOVED);

    ASSERT_EQUAL(fdA.size(), 2);
    ASSERT(fdB.empty());
    ASSERT_EQUAL(fdI.size(), 1);
    ASSERT_EQUAL(fdR.size(), 1);
}

void TOfRelevance() {
    SearchServer server = GetSearchServer();
    const std::vector<Document>& fd = server.FindTopDocuments("-1word2 -2word1 3word1 3word2 3word3 4word1 5word5 5word2 6word3 6word1");
    ASSERT_EQUAL(fd[0].relevance, 0.89587973461402748);
    ASSERT_EQUAL(fd[1].relevance, 0.29862657820467581);
}

void TGetSearchServer() {
    RUN_TEST(TExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TAddingDocuments);
    RUN_TEST(TStopWords);
    RUN_TEST(TMinusWords);
    RUN_TEST(TMatchingDocuments);
    RUN_TEST(TSortByRelevance);
    RUN_TEST(TOfRating);
    RUN_TEST(TSortingByPredicate);
    RUN_TEST(TByStatus);
    RUN_TEST(TOfRelevance);
}

int main() {
    TGetSearchServer()();
    std::cout << "Search server testing finished" << std::endl;
}
