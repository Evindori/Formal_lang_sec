#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <tuple>
#include <vector>
#include <set>
#include <map>
#include <fstream>


using namespace std;
class Algo {
public:
	Algo(vector<pair<char, string>> grammar, set<char> grammar_symbols, char start_symbol);
	void fit();//grammar содержится в самом algo, поэтому в ее передаче в качестве ар-та нет нужды
	bool predict(string s);

private:
	typedef tuple<int, int, char> element;
	vector<pair<char, string>> grammar_;
	vector<set<element>> states_;
	map<char, set<char>> first_;
	map<pair<int, char>, pair<int, int>> action_;
	map<pair<int, char>, pair<int, int>> step_;
	set<element> step(const set<element>& state, char c);
	set<char> grammar_symbols_;

	char start_symbol_;
	void grammar_check(int x);
	void closing(set<element>& state);
	void first_compution();
};

Algo::Algo(vector<pair<char, string>> grammar, set<char> grammar_symbols, char start_symbol) :
	grammar_{ grammar },
	grammar_symbols_{ grammar_symbols },
	start_symbol_{ start_symbol }
{
}


void Algo::first_compution() {
	bool not_enogh = true;
	for (auto symbol : grammar_symbols_) {first_[symbol] = set<char>();}
	while (not_enogh) {
		not_enogh = false;
		for (auto rule : grammar_) {
			char first_r = rule.first;
			string second_r = rule.second;
			set<char> prev_set = first_[first_r];
			if (second_r == "") {
				first_[first_r].insert('\0');
				if (prev_set != first_[first_r]) {not_enogh = true;}
				continue;
			}
			if (grammar_symbols_.find(second_r[0]) == grammar_symbols_.end()) {
				first_[first_r].insert(second_r[0]);
				if (prev_set != first_[first_r]) {not_enogh = true;}
				continue;
			}
			for (auto c : first_[second_r[0]]) {
				if (c != '\0') {first_[first_r].insert(c);}
			}
			int i = 0;
			while (first_[second_r[i]].count('\0') && i < second_r.length() - 1) {
				if (grammar_symbols_.find(second_r[i + 1]) != grammar_symbols_.end()) {
					for (auto c : first_[second_r[i + 1]]) {
						if (c != '\0') {first_[first_r].insert(c);}
					}
					i++;
				}
				else {
					first_[first_r].insert(second_r[i + 1]);
					break;
				}
			}
			if (grammar_symbols_.find(second_r[i]) != grammar_symbols_.end()) {
				if (first_[second_r[i]].count('\0') && i == second_r.length() - 1) {first_[first_r].insert('\0');}
			}

			if (prev_set != first_[first_r]) {not_enogh = true;}
		}
	}
}

void Algo::closing(set<Algo::element>& state) {
	size_t size = 0;
	while (state.size() > size) {
		size = state.size();
		for (auto item : state) {
			auto curr_rule = grammar_[get<0>(item)];
			if (curr_rule.second.length() == get<1>(item)) { continue; }
			for (int i = 0; i < grammar_.size(); i++) {
				auto rule = grammar_[i];
				if (rule.first == curr_rule.second[get<1>(item)]) {
					if (curr_rule.second.length() == get<1>(item) + 1) {
						state.insert(make_tuple(i, 0, get<2>(item)));
						continue;
					}
					if (grammar_symbols_.find(curr_rule.second[get<1>(item) + 1]) == grammar_symbols_.end()) {
						state.insert(make_tuple(i, 0, curr_rule.second[get<1>(item) + 1]));
						continue;
					}
					for (auto c : first_[curr_rule.second[get<1>(item) + 1]]) {state.insert(make_tuple(i, 0, c));}
				}
			}
		}
	}
}

set<Algo::element> Algo::step(const set<Algo::element>& state, char c) {
	set<Algo::element> new_state;
	for (auto item : state) {
		auto curr_rule = grammar_[get<0>(item)];
		if (curr_rule.second.length() == get<1>(item)) {continue;}
		if (curr_rule.second[get<1>(item)] == c) {new_state.insert(make_tuple(get<0>(item), get<1>(item) + 1, get<2>(item)));}
	}
	closing(new_state);
	return new_state;
}


void Algo::grammar_check(int i) {
	for (auto rule : states_[i]) {
		if (grammar_[get<0>(rule)].first == start_symbol_ && get<1>(rule) == 1) {
			if (action_.find(make_pair(i, '\0')) != action_.end()) {
				if (action_[make_pair(i, '\0')] != make_pair(2, -1)) {throw logic_error("Wrong grammar");}
			}
			else {action_[make_pair(i, '\0')] = make_pair(2, -1);}
		}
	}
	for (char c = -127; c < 127; c++) {
		for (auto rule : states_[i]) {
			if (get<1>(rule) < grammar_[get<0>(rule)].second.length()) {
				char a = grammar_[get<0>(rule)].second[get<1>(rule)];
				if (a != c) { continue;}
				if (!grammar_symbols_.count(a)) {
					for (int j = 0; j < states_.size(); j++) {
						if (step(states_[i], a) == states_[j]) {
							if (action_.find(make_pair(i, a)) != action_.end()) {
								if (action_[make_pair(i, a)] != make_pair(0, j)) { throw logic_error("Wrong grammar");}
							}
							else {
								action_[make_pair(i, a)] = make_pair(0, j);
								break;
							}
						}
					}
				}
				else
					continue;
			}
			else {
				char a = get<2>(rule);
				if (a != c) {continue;}
				if (grammar_[get<0>(rule)].first == start_symbol_) {continue;}
				if (!grammar_symbols_.count(a)) {
					if (action_.find(make_pair(i, a)) != action_.end()) {
						if (action_[make_pair(i, a)] != make_pair(1, get<0>(rule))) { throw logic_error("Wrong grammar");}
					}
					else { action_[make_pair(i, a)] = make_pair(1, get<0>(rule)); }
				}
			}
		}
	}
}
void Algo::fit() {
	set<element> start;
	int i = 0;
	first_compution();
	for (int i = 0; i < grammar_.size(); i++) {
		auto r = grammar_[i];
		if (r.first == start_symbol_) {start.insert(make_tuple(i, 0, '\0'));}
	}
	closing(start);
	states_.push_back(start);
	while (i < states_.size()) {
		auto curr_state = states_[i];
		for (char c = -127; c < 127; c++) {
			auto state = step(curr_state, c);
			if (!state.empty()) {
				for (auto s : states_) {
					if (s == state) {return;}
				}
				states_.push_back(state);
			}
		}
		i++;
	}
	for (int i = 0; i < states_.size(); i++) {
		grammar_check(i);
		for (char c = -127; c < 127; c++) {
			if (action_.find(make_pair(i, c)) == action_.end()) {action_[make_pair(i, c)] = make_pair(3, -1);}
		}
	}
	for (int i = 0; i < states_.size(); i++) {
		for (auto a : grammar_symbols_) {
			for (int j = 0; j < states_.size(); j++) {
				if (step(states_[i], a) == states_[j]) {step_[make_pair(i, a)] = make_pair(0, j);}
			}
			if (step_.find(make_pair(i, a)) == step_.end()) {step_[make_pair(i, a)] = make_pair(1, -1);}
		}
	}
}

bool Algo::predict(string s) {
	vector<pair<int, char>> helping_stack;
	int position = 0;
	s += '\0';
	helping_stack.push_back(make_pair(0, '\0'));
	while (position <= s.length()) {
		char item = s[position];
		pair<int, char> back_ = helping_stack.back();
		if (action_[make_pair(back_.first, item)].first == 0) {
			helping_stack.push_back(make_pair(action_[make_pair(back_.first, item)].second, item));
			position++;
			continue;
		}
		if (action_[make_pair(back_.first, item)].first == 1) {
			pair<char, string> rule = grammar_[action_[make_pair(back_.first, item)].second];
			for (int i = 0; i < rule.second.length(); i++) {helping_stack.pop_back();}
			int q_new = helping_stack.back().first;
			char first_r = rule.first;
			if (step_[make_pair(q_new, first_r)].first == 0) {helping_stack.push_back(make_pair(step_[make_pair(q_new, first_r)].second, first_r));}
			else {break;}
			continue;
		}
		if (action_[make_pair(back_.first, item)].first == 2) {return true;}
		if (action_[make_pair(back_.first, item)].first == 3) {break;}
	}
	return false;
}

void ans(Algo parser,int cont ) {
	for (int i = 0; i < cont; i++) {
		string word;
		getline(cin, word);
		bool result = parser.predict(word);
		if (result) { cout << "Correct"; }
		else { cout << "Incorrect"; }
	}
}

int main(int argc, char* argv[]) {
	int cont;
	char start_symbol;
	char left;
	string file, symbols, rule, right;
	string::size_type sz;
	vector<pair<char, string>> grammar;
	set<char> grammar_symbols;
		
	file = argv[1];
	cont = stoi(argv[2], &sz);
	ifstream input;
	input.open(file);
	input >> start_symbol;
	input >> symbols;
	rule = "";

	while (!input.eof()) {
		input >> rule;
		left = rule[0];
		right = rule.substr(3);
		grammar.push_back(make_pair(left, right));
	}
	for (auto c : symbols) {grammar_symbols.insert(c);}
	Algo parser = Algo(grammar, grammar_symbols, start_symbol);
	parser.fit();
	ans(parser, cont);
}
