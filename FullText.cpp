// FullText.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;
#include <fstream>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>

class Solver {
	class Mydatabase {
	public:
		Mydatabase(string id, string title, string& words) :id_(id), title_(title),
			words_(words) {};
		string id_;
		string title_;
		string words_;
	};

public:
	list<Mydatabase> mainData;
	using databaseIterator_t = list<Mydatabase>::iterator;
	databaseIterator_t cIterator;
	set<string> cSet;
	//using dataItem_t = list < pair<int, databaseIterator_t>>;
	using dataItem_t = pair<string, list<databaseIterator_t>>;
	using store_t = list<dataItem_t>;
	using wordToArticleIndex_t = unordered_map<string, store_t::iterator>;
	store_t store;
	wordToArticleIndex_t wordToArticleIndex;
	string word;

	
	void processIt(char c,int charCount) {
		if (isalpha(c)) {
			if (c >= 'A' && c <= 'Z') {
				c += 'a' - 'A';
			}

			word += c;
		}
		else if (word != "")
		{
			addWord(word,charCount);
			word = "";
		}
	}
	void addWord(string word,int charCount) {
		if (cSet.find(word) == cSet.end()) {
			cSet.emplace(word);
			auto existing = wordToArticleIndex.find(word);
			if (existing != wordToArticleIndex.end() && existing->second != store.end())
			{
				existing->second->second.push_back(cIterator);
				//cout << cIterator->id_;
			}
			else
			{
				list<databaseIterator_t> newList;
				newList.push_back(cIterator);
				//cout << cIterator->id_;
				dataItem_t newPair(word, newList);
				store.push_back(newPair);

				auto newItem = store.end();
				newItem--;

				wordToArticleIndex.emplace(word, newItem);
			}
		}
	}
	void readWords(string& thirdLine) {
		char c;
		int charCount=0;
		for (auto i = thirdLine.begin(); i != thirdLine.end(); i++)
		{
			
			c = *i;
			processIt(c,charCount); //todo charcount
			charCount += 1;
		}
		addWord(word,charCount);
	}
	
	void readArticles(string& articles) {

		ifstream myfile(articles);
		if (myfile.is_open())
		{
			string cID;
			string cTitle;
			string cText;
			string line;
			getline(myfile, line);
			while (line != "")
			{
				cSet.clear();
				cID = line;
				getline(myfile, line);
				cTitle = line;
				getline(myfile, line);
				cText = line;
				mainData.push_back(Mydatabase{cID,cTitle,cText});
				cIterator = mainData.end();
				cIterator--;
				readWords(line);
				
				
				getline(myfile, line);
			}
			myfile.close();
		}
	}

	void find(string word) {
		auto iteratorIntoWordToArticleIndex = wordToArticleIndex.find(word);
		if (iteratorIntoWordToArticleIndex != wordToArticleIndex.end() &&
			iteratorIntoWordToArticleIndex->second != store.end()){
			auto path = iteratorIntoWordToArticleIndex->second->second;
			for (auto&& item : path) {
					cout << item->id_;			
			}
		}
	}
};
int main(int argc, char** argv)
{
	Solver solv;
	string articles = argv[1];
	solv.readArticles(articles);
	
	solv.find("to");
	string commands;
	if (argc > 2) { commands = argv[2]; }
	

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
