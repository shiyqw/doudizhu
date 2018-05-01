#include <iostream>
#include <vector>
#include <set>

#include "jsoncpp/json.h"

using namespace std;

set<int> my_cards;
vector<int> last_hand;

int my_shape[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int remain_shape[15] = {4,4,4,4,4,4,4,4,4,4,4,4,4,1,1};
int remain_card_number[3] = {20,17,17};
int my_position;

void init() {
    ;
}

vector<int> play() {
    vector<int> dummy;
    return dummy;
}

int card_to_point(int card) {
    if (card >= 52) {
	return card - 39;
    } else {
	return card / 4;
    }
}


void input() {
    string line;
    getline(cin, line);
    Json::Value input;
    Json::Reader reader;
    reader.parse(line, input);

    cout << input << endl; // for debug

    // First request
    {
	auto first_request = input["requests"][0u]; // 
	auto own = first_request["own"];
	//auto dipai = first_request["public"];
	auto first_history = first_request["history"];
	for (unsigned i = 0; i < own.size(); i++) {
	    int card = own[i].asInt();
	    my_cards.insert(card);
	    ++my_shape[card_to_point(card)];
	    --remain_shape[card_to_point(card)];
	}
	// Get my position
	if (first_history[0u].size() == 0) {
	    if (first_history[1].size() == 0) {
		my_position = 0; // Dizhu
	    } else {
		my_position = 1; // Xiajia
	    }
	} else {
	    my_position = 2; // Menban
	}
    }

    int turn_number = input["requests"].size();

    for (int turn = 0; turn < turn_number; ++turn) {
	auto history = input["requests"][turn]["history"]; 

	// Recover Remain Shape and Remain Card Numbers
	for (int pos = 0; pos < 2; pos++) {
	    int player_position = (my_position + pos + 1) % 3;
	    auto player_action = history[pos]; 
	    for (int i = 0; i < player_action.size(); i++) {
		int card = player_action[i].asInt(); 
		--remain_shape[card_to_point(card)];
	    }
	    remain_card_number[player_position] -= player_action.size();
	}

	if (turn != turn_number - 1) {
	    // Recover My Shape and My Card and My Card Numbers
	    auto my_action = input["responses"][turn]; 
	    for (int i = 0; i < my_action.size(); i++) {
		int card = my_action[i].asInt(); 
		--my_shape[card_to_point(card)];
		my_cards.erase(card); 
	    }
	    remain_card_number[my_position] -= my_action.size();
	} else {
	    // Record last hand
	    for (int pos = 1; pos >= 0; --pos) {
		if (history[pos].size() != 0) {
		    for (int i = 0; i < history[pos].size(); ++i) {
			last_hand.push_back(history[pos][i].asInt());
		    }
		    break;
		}
	    }
	}
    }
}

/**
 * output result
 */
void output(vector<int> cards) {
    Json::Value result, response(Json::arrayValue);
    for (auto card : cards) {
	response.append(card);
    }
    result["response"] = response;
    Json::FastWriter writer;
    cout << writer.write(result) << endl;
}

int main() {
    init();
    input();
    cout << my_position << endl;
    /** Below for debug **/
    for (int i  = 0; i < 15; ++i) {
        cout << my_shape[i] << ' ';
    }
    cout << endl;
    for (int i  = 0; i < 15; ++i) {
        cout << remain_shape[i] << ' ';
    }
    cout << endl;
    for (int i  = 0; i < 3; ++i) {
        cout << remain_card_number[i] << ' ';
    }
    cout << endl;
    /** Above for debug **/
    output(play());
}
