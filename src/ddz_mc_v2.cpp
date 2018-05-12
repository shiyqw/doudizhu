#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <functional>
#include <ctime>
#include <cstdlib>

#include "jsoncpp/json.h"

using namespace std;

int mc_game_number;

set<int> my_cards;
set<int> last_hand_cards;
vector<int> last_hand_shape;
vector<int> my_shape = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int remain_shape[15] = {4,4,4,4,4,4,4,4,4,4,4,4,4,1,1};
int rem[3] = {20,17,17};
int mc_rem[3];
int mc_shape[3][15];
int my_pos;

/** Begin part for evaluation **/
double weight[3][23] = { {
    0.10221,
    0.010115,
    0.038082,
    -0.028686,
    -0.011645,
    0.02509,
    0.094062,
    0.023078,
    0.05892,
    0.14387,
    0.24303,
    0.38371,
    0.53677,
    1.2718,
    1.5603,
    0.031269,
    0.078009,
    0.048704,
    0.1709,
    0.048337,
    -0.018812,
    -0.67239,
    0.26338
},{
    0.53871,
    0.015115,
    0.047219,
    0.042448,
    0.06138,
    0.059423,
    0.14273,
    0.10151,
    0.11295,
    0.19494,
    0.28551,
    0.35965,
    0.52937,
    0.72966,
    0.91706,
    0.12818,
    -0.13026,
    -0.060856,
    0.089493,
    0.023988,
    -0.010355,
    -0.23391,
    0.2827
},{
    0.91588,
    0.105,
    0.11169,
    0.11221,
    0.11908,
    0.1251,
    0.12264,
    0.080705,
    0.092844,
    0.17455,
    0.24032,
    0.29131,
    0.39006,
    0.54337,
    0.64612,
    0.10135,
    -0.10151,
    -0.093306,
    0.10316,
    0.027836,
    0.018279,
    -0.24011,
    0.047457
}};

double mc_evaluate(int pos) {
    int last_zero = -1;
    int single_string_length = 0;
    int double_string_length = 0;
    int triple_string_length = 0;

    // Get Single String Length
    for (int j = 0; j < 13; ++j) {
	if (mc_shape[pos][j] == 0 || j == 12) {
	    if (j - 1 - last_zero >= 5) {
		if (single_string_length < j - 1 - last_zero) {
		    single_string_length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }

    // Get Double String Length
    last_zero = -1;
    for (int j = 0; j < 13; ++j) {
	if (mc_shape[pos][j] < 2 || j == 12) {
	    if (j - 1 - last_zero >= 3) {
		if (double_string_length < j - 1 - last_zero) {
		    double_string_length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }

    // Get Triple String Length
    last_zero = -1;
    for (int j = 0; j < 13; ++j) {
	if (mc_shape[pos][j] < 3 || j == 12) {
	    if (j - 1 - last_zero >= 2) {
		if (triple_string_length < j - 1 - last_zero) {
		    triple_string_length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }

    // Get Non Zero Num and Bomb Num
    int non_zero_num = 0;
    int has_bomb = 0;
    for (int j = 0; j < 15; ++j) {
	if (mc_shape[pos][j] != 0) {
	    ++non_zero_num;
	}
	if (mc_shape[pos][j] == 4) {
	    ++has_bomb;
	}
    }
    if (mc_shape[pos][14] == 1 && mc_shape[pos][13] == 1) {
	++has_bomb;
    }

    // Get All features
    vector<double> features;
    features.push_back(1.0);
    for (int i = 1; i < 15; ++i) {
	features.push_back(double(mc_shape[pos][i]));
    }
    for (int i = 0; i < 3; ++i) {
	features.push_back(double(rem[i]));
    }
    features.push_back(double(single_string_length));
    features.push_back(double(double_string_length));
    features.push_back(double(triple_string_length));
    features.push_back(double(non_zero_num));
    features.push_back(double(has_bomb));

    /** output features (for debug) **/
    //cout << "features: ";
    //for (auto feature : features) {
    //    cout << feature << ',';
    //}
    /** end output features **/
    // Get Weighted Sum
    double value = 0.;
    for (int i = 0; i < features.size(); ++i) {
	value += features[i] * weight[my_pos][i];
    } 
    //cout << " v:" << value << endl;
    return value;
}

double evaluate() {
    int last_zero = -1;
    int single_string_length = 0;
    int double_string_length = 0;
    int triple_string_length = 0;

    // Get Single String Length
    for (int j = 0; j < 13; ++j) {
	if (my_shape[j] == 0 || j == 12) {
	    if (j - 1 - last_zero >= 5) {
		if (single_string_length < j - 1 - last_zero) {
		    single_string_length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }

    // Get Double String Length
    last_zero = -1;
    for (int j = 0; j < 13; ++j) {
	if (my_shape[j] < 2 || j == 12) {
	    if (j - 1 - last_zero >= 3) {
		if (double_string_length < j - 1 - last_zero) {
		    double_string_length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }

    // Get Triple String Length
    last_zero = -1;
    for (int j = 0; j < 13; ++j) {
	if (my_shape[j] < 3 || j == 12) {
	    if (j - 1 - last_zero >= 2) {
		if (triple_string_length < j - 1 - last_zero) {
		    triple_string_length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }

    // Get Non Zero Num and Bomb Num
    int non_zero_num = 0;
    int has_bomb = 0;
    for (int j = 0; j < 15; ++j) {
	if (my_shape[j] != 0) {
	    ++non_zero_num;
	}
	if (my_shape[j] == 4) {
	    ++has_bomb;
	}
    }
    if (my_shape[14] == 1 && my_shape[13] == 1) {
	++has_bomb;
    }

    // Get All features
    vector<double> features;
    features.push_back(1.0);
    for (int i = 1; i < 15; ++i) {
	features.push_back(double(my_shape[i]));
    }
    for (int i = 0; i < 3; ++i) {
	features.push_back(double(rem[i]));
    }
    features.push_back(double(single_string_length));
    features.push_back(double(double_string_length));
    features.push_back(double(triple_string_length));
    features.push_back(double(non_zero_num));
    features.push_back(double(has_bomb));

    /** output features (for debug) **/
    //cout << "features: ";
    //for (auto feature : features) {
    //    cout << feature << ',';
    //}
    /** end output features **/
    // Get Weighted Sum
    double value = 0.;
    for (int i = 0; i < features.size(); ++i) {
	value += features[i] * weight[my_pos][i];
    } 
    //cout << " v:" << value << endl;
    return value;
}

/** End part for evaluation **/

struct Hand {
    int point;
    int length;
    int width;
    vector<int> carry;
    Hand() {
	point = 0;
	length = 0;
	width = 0;
	carry.clear();
    }
    Hand(int p, int w, int l) {
	point = p;
	width = w;
	length = l;
	carry.clear();
    }
    void set_carry(vector<int> c) {
	carry = c;
    }
    void clear_carry() {
	carry.clear();
    }
    bool is_pass() {
	return length == 0;
    }
    bool is_bomb() {
	if (width == 4 && length == 1 && carry.empty()) {
	    return true;
	} 
	if (width == 1 && point == 13 && length == 2 && carry.empty()) {
	    return true;
	}
	return false;
    }
    void show() {
	cout << point << ":" << width << "x" << length;
	for (auto card : carry) {
	    cout << " " << card;
	}
	cout << endl;
    }
};

Hand last_hand;
/** Begin Type transform Functions
 * Note: All card-related transform functions are only used by me
 * Others change (if we need in monte-carlo) only use Shape->Hand
 * **/
int card_to_point(int card) {
    if (card >= 52) {
	return card - 39;
    } else {
	return card / 4;
    }
}

vector<int> card_to_shape(set<int> cards) {
    vector<int> shape(15, 0);
    for (auto card : cards) {
	++shape[card_to_point(card)];
    }
    return shape;
}

// Note: this method will only used to transform last hand
Hand shape_to_hand(vector<int> shape) {
    int width = *max_element(shape.begin(), shape.end());
    int total = accumulate(shape.begin(), shape.end(), 0);
    if (width == 4) {
	vector<int> carry;
	Hand hand;
	for (int i = 0; i < 15; ++i) {
	    if (shape[i] == 4) {
		hand = Hand(i, 4, 1);
	    } else {
		for (int j = 0; j < shape[i]; ++j) {
		    carry.push_back(i);
		}
	    }
	}
	hand.set_carry(carry);
	return hand;
    } else if (width > 0) {
	int left = -1;
	int right = 15;
	for (int i = 0; i < 15; ++i) {
	    if (shape[i] == width) {
		left = i;
		break;
	    }
	}
	for (int i = left+1; i < 15; ++i) {
	    if (shape[i] != width) {
		right = i;
		break;
	    }
	}
	Hand hand = Hand(left, width, right-left);
	if (width != 3) {
	    return hand;
	}
	vector<int> carry;
	for (int i = 0; i < 14; ++i) {
	    if (shape[i] != 0 && shape[i] != 3) {
		for (int j = 0; j < shape[i]; ++j) {
		    carry.push_back(i);
		}
	    }
	}
	hand.set_carry(carry);
	return hand;
    }
    return Hand();
}

Hand card_to_hand(set<int> cards) {
    return(shape_to_hand(card_to_shape(cards)));
}

vector<int> hand_to_shape(Hand hand) {
    vector<int> shape(15, 0);
    for (int i = hand.point; i < hand.point+hand.length; ++i) {
	shape[i] += hand.width;
    }
    for (auto point : hand.carry) {
	shape[point]++;
    }
    return shape;
}

vector<int> shape_to_card(vector<int> shape) {
    vector<int> cards;
    for (auto card : my_cards) {
	auto point = card_to_point(card);
	if (shape[point] > 0) {
	    cards.push_back(card);
	    shape[point]--;
	}
    }
    return cards;
}

vector<int> hand_to_card(Hand hand) {
    return shape_to_card(hand_to_shape(hand));
}

/** End Type transform functions **/

void mc_redo(int pos, Hand hand) {
    if (hand.is_pass()) {
	return;
    } 
    for (int i = hand.point; i < hand.point + hand.length; ++i) {
	mc_shape[pos][i] -= hand.width;
	mc_rem[pos] -= hand.width;
    }
    for (auto card : hand.carry) {
	mc_shape[pos][card]--;
	mc_rem[pos]--;
    }
}

void mc_undo(int pos, Hand hand) {
    if (hand.is_pass()) {
	return;
    } 
    for (int i = hand.point; i < hand.point + hand.length; ++i) {
	mc_shape[pos][i] += hand.width;
	mc_rem[pos] += hand.width;
    }
    for (auto card : hand.carry) {
	mc_shape[pos][card]++;
	mc_rem[pos]++;
    }
}

void redo(Hand hand) {
    if (hand.is_pass()) {
	return;
    } 
    for (int i = hand.point; i < hand.point + hand.length; ++i) {
	my_shape[i] -= hand.width;
	rem[my_pos] -= hand.width;
    }
    for (auto card : hand.carry) {
	my_shape[card]--;
	rem[my_pos]--;
    }
}

void undo(Hand hand) {
    if (hand.is_pass()) {
	return;
    } 
    for (int i = hand.point; i < hand.point + hand.length; ++i) {
	my_shape[i] += hand.width;
	rem[my_pos] += hand.width;
    }
    for (auto card : hand.carry) {
	my_shape[card]++;
	rem[my_pos]++;
    }
}

bool check_valid(Hand hand, Hand prev) {
    // special cases
    if (prev.is_pass()) {
	return true;
    } 
    if (prev.is_bomb()) {
	if (!hand.is_bomb()) {
	    return false;
	} else {
	    return hand.point > prev.point;
	}
    }
    if (hand.is_bomb()) {
	return true;
    }
    if (hand.width != prev.width) {
	return false;
    }
    if (hand.length != prev.length) {
	return false;
    }
    if (hand.carry.size() != prev.carry.size()) {
	return false;
    }
    // regular cases
    return (hand.point > prev.point);
}
    
void mc_init() {
    for (int i = 0; i < 3; ++i) {
	mc_rem[i] = rem[i];
    }
    // Shuffle remaining cards;
    for (int i = 0; i < 15; ++i) {
	mc_shape[my_pos][i] = my_shape[i];
    }
    for (int i = 1; i < 3; ++i) {
	for (int j = 0; j < 15; ++j) {
	    mc_shape[(my_pos+i)%3][j] = 0;
	}
    }
    vector<int> cards;
    for (int i = 0; i < 15; ++i) {
	for (int j = 0; j < remain_shape[i]; ++j) {
	    cards.push_back(i);
	}
    }
    random_shuffle(cards.begin(), cards.end());
    for(int i = 0; i < rem[(my_pos+1)%3]; ++i) {
	mc_shape[(my_pos+1)%3][cards[i]]++;
    }
    for(int i = 0; i < rem[(my_pos+2)%3]; ++i) {
	mc_shape[(my_pos+2)%3][cards[i+rem[(my_pos+1)%3]]]++;
    }
}

int mc_get_length_with_width(int pos, int left, int width) {
    int right_limit = width == 3 ? 13 : 12;
    for (int i = left; i <= right_limit; ++i) {
	if (mc_shape[pos][i] < width || i == right_limit) {
	    return (i-left);
	}
    }
    return 0;
}
//
// Helper function to generate combination
// Using for generate carried cards.
vector<vector<int>> generate_combination(int n, int r) {
    vector<vector<int>> combinations;
    vector<bool> v(n);
    fill(v.end() - r, v.end(), true);
    do {
	vector<int> c;
        for (int i = 0; i < n; ++i) {
            if (v[i]) {
               c.push_back(i); 
            }
        }
	combinations.push_back(c);
    } while (std::next_permutation(v.begin(), v.end()));
    return combinations;
}


Hand mc_step(int pos, Hand prev) {
    auto max_hand = Hand();
    double max_val = -10000.;
    /** Search Strings, 3+2 and 3+1 **/
    if (prev.length == 0 || prev.length * prev.width >= 3) {
	for (auto left = 0; left < 13; ++left) {
	    for (auto width = 1; width < 4; ++width) {
		auto length = mc_get_length_with_width(pos, left, width);
		auto min_length = 7-2*width;
		for (auto l = min_length; l <= length; ++l) {
		    Hand string_hand = Hand(left, width, l);
		    //cout << "mc" << endl;
		    //string_hand.show();
		    // Add carriables for 3-string or pure 3
		if (width == 3) {
		    vector<vector<int>> carriables;
		    for (auto carry_width = 1; carry_width < width; ++carry_width) {
			vector<int> carriable;
			for (auto i = 0; i < 15; ++i) {
			    if (i >= left && i < left+l) {
				continue;
			    }
			    if (mc_shape[pos][i] < carry_width) {
				continue;
			    }
			    carriable.push_back(i);
			}
			carriables.push_back(carriable);
		    }

			for(auto carry_width = 1; carry_width < width; ++carry_width) {
			    auto carriable = carriables[carry_width-1];
			    if (carriable.size() < length) {
				continue;
			    } 
			    auto combinations = generate_combination(carriable.size(), l);
			    for (auto combination : combinations) {
				vector<int> carry;
				for (auto index : combination) {
				    for (int j = 0; j < carry_width; ++j) {
					carry.push_back(carriable[index]);
				    }
				}
				string_hand.set_carry(carry);
				//string_hand.show();
				/** Start main evaluation area **/
				if (check_valid(string_hand, prev)) {
				    //evaluate();
				    mc_redo(pos, string_hand);
				    //double val = 0.;
				    double val = mc_evaluate(pos);
				    //string_hand.show();
				    if (val > max_val) {
					max_hand = string_hand;
					max_val = val;
				    }
				    mc_undo(pos, string_hand);
				}
				/** End main evaluation area **/
				string_hand.clear_carry();
			    }
			}
		    }

		    if (!check_valid(string_hand, prev)) {
			continue;
		    }
		    //string_hand.show();
		    mc_redo(pos, string_hand);
		    // Check pure string
		    auto val = mc_evaluate(pos);
		    if (val > max_val) {
			max_hand = string_hand;
			max_val = val;
		    }
		    mc_undo(pos, string_hand);
		}
	    }
	}
    }

    /** Search pure 1 or 2  card hands **/
    if (prev.length <= 1) {
	for (int i = 0; i < 15; ++i) {
	    for (int width = 1; width <= mc_shape[pos][i] && width <= 2; ++width) {
		Hand pure_hand = Hand(i, width, 1);
		//pure_hand.show();
		if (check_valid(pure_hand, prev)) {
		    mc_redo(pos, pure_hand);
		    double val = mc_evaluate(pos);
		    if (val > max_val) {
			max_val = val;
			max_hand = pure_hand;
		    }
		    mc_undo(pos, pure_hand);
		}
	    }
	}
    }

    /** Search Bomb **/
    if (rem[0] <= 6 || rem[1] <= 6 || rem[2] <= 6) {
	for (int i = 0; i < 13; ++i) {
	    if (mc_shape[pos][i] == 4) {
		Hand bomb_hand = Hand(i, 4, 1);
		//bomb_hand.show();
		if (check_valid(bomb_hand, prev)) {
		    mc_redo(pos, bomb_hand);
		    double val = mc_evaluate(pos);
		    if (val > max_val) {
			max_val = val;
			max_hand = bomb_hand;
		    }
		    mc_undo(pos, bomb_hand);
		}
	    }
	}
	if (mc_shape[pos][13] == 1 && mc_shape[pos][14] == 1) {
	    Hand bomb_hand = Hand(13, 1, 2);
	    //bomb_hand.show();
	    if (check_valid(bomb_hand, prev)) {
		mc_redo(pos, bomb_hand);
		double val = mc_evaluate(pos);
		if (val > max_val) {
		    max_val = val;
		    max_hand = bomb_hand;
		}
		mc_undo(pos, bomb_hand);
	    }
	}
    }

    /** TODO: Search 4+1+1 or 4+2+2 **/

    /** Search Pass (May be used for cooperation) **/
    //double val = evaluate();
    //if (val > max_val) {
    //    max_val = val;
    //    max_hand = Hand();
    //}

    //cout << "final result :" << endl;
    //cout << pos << " do " ;
    //max_hand.show();
    mc_redo(pos, max_hand);
    return max_hand;
}


int mc_run(Hand prev_hand) {
    //cout << endl;
    //cout << "start run" << endl;
    //prev_hand.show();
    //for (int i = 0; i < 3; ++i) {
    //    for (int j = 0; j < 15; ++j) {
    //        cout << mc_shape[i][j] << " ";
    //    }
    //    cout << endl;
    //}
    //cout << "---------------------" << endl;
    int position = (my_pos + 1) % 3;
    int pass = 0;
    while(mc_rem[0] != 0 && mc_rem[1] != 0 && mc_rem[2] != 0) {
	Hand hand = mc_step(position, prev_hand);
	if (hand.length == 0) {
	    pass += 1;
	} else {
	    prev_hand = hand;
	    pass = 0;
	}
	if (pass == 2) {
	    prev_hand = Hand();
	}
	position = (position + 1) % 3;
	//for (int i = 0; i < 3; ++i) {
	//    for (int j = 0; j < 15; ++j) {
	//	cout << mc_shape[i][j] << " ";
	//    }
	//    cout << endl;
	//}
	//cout << "---------------------" << endl;
    }
    int vic_pos;
    if(mc_rem[0] == 0) {
	vic_pos = 0;
    } else if (mc_rem[1] == 0) {
	vic_pos = 1;
    } else if (mc_rem[2] == 0) {
	vic_pos = 2;
    }
    //cout << "winner: " << vic_pos << endl;
    return vic_pos;
}

void init() {
    ;
}

vector<int> follow_play() {
    vector<int> dummy;
    return dummy;
}

vector<int> new_play() {
    vector<int> dummy;
    return dummy;
}


bool to_final() {
    //TODO: If reaching final, we need mc search
    return false;
}

int get_length_with_width(int left, int width) {
    int right_limit = width == 3 ? 13 : 12;
    for (int i = left; i <= right_limit; ++i) {
	if (my_shape[i] < width || i == right_limit) {
	    return (i-left);
	}
    }
    return 0;
}


//vector<int> play(Hand prev) {
//    if (to_final()) {
//	return mc_search();
//    }
//    auto max_hand = Hand();
//    double max_val = -10000.;
//    /** Search Strings, 3+2 and 3+1 **/
//    for (auto left = 0; left < 13; ++left) {
//	for (auto width = 0; width < 4; ++width) {
//	    auto length = get_length_with_width(left, width);
//	    auto min_length = 7-2*width;
//	    for (auto l = min_length; l <= length; ++l) {
//		Hand string_hand = Hand(left, width, l);
//		// Add carriables for 3-string or pure 3
//		if (width == 3) {
//		    vector<vector<int>> carriables;
//		    for (auto carry_width = 1; carry_width < width; ++carry_width) {
//			vector<int> carriable;
//			for (auto i = 0; i < 15; ++i) {
//			    if (i >= left && i < left+l) {
//				continue;
//			    }
//			    if (my_shape[i] < carry_width) {
//				continue;
//			    }
//			    carriable.push_back(i);
//			}
//			carriables.push_back(carriable);
//		    }
//
//		    for(auto carry_width = 1; carry_width < width; ++carry_width) {
//			auto carriable = carriables[carry_width-1];
//			if (carriable.size() < length) {
//			    continue;
//			} 
//			//string_hand.show();
//			auto combinations = generate_combination(carriable.size(), l);
//			for (auto combination : combinations) {
//			    vector<int> carry;
//			    for (auto index : combination) {
//				for (int j = 0; j < carry_width; ++j) {
//				    carry.push_back(carriable[index]);
//				}
//			    }
//			    string_hand.set_carry(carry);
//			    //string_hand.show();
//			    /** Start main evaluation area **/
//			    if (check_valid(string_hand, prev)) {
//				//evaluate();
//				redo(string_hand);
//				//double val = 0.;
//				double val = evaluate();
//				//string_hand.show();
//				if (val > max_val) {
//				    max_hand = string_hand;
//				    max_val = val;
//				}
//				undo(string_hand);
//			    }
//			    /** End main evaluation area **/
//			    string_hand.clear_carry();
//			}
//		    }
//		}
//
//		if (!check_valid(string_hand, prev)) {
//		    continue;
//		}
//		//string_hand.show();
//		redo(string_hand);
//		// Check pure string
//		auto val = evaluate();
//		if (val > max_val) {
//		    max_hand = string_hand;
//		    max_val = val;
//		}
//		undo(string_hand);
//	    }
//	}
//    }
//
//    /** Search pure 1 or 2  card hands **/
//    for (int i = 0; i < 15; ++i) {
//	for (int width = 1; width <= my_shape[i] && width <= 2; ++width) {
//	    Hand pure_hand = Hand(i, width, 1);
//	    //pure_hand.show();
//	    if (check_valid(pure_hand, prev)) {
//		redo(pure_hand);
//		double val = evaluate();
//		if (val > max_val) {
//		    max_val = val;
//		    max_hand = pure_hand;
//		}
//		undo(pure_hand);
//	    }
//	}
//    }
//
//    /** Search Bomb **/
//    if (rem[0] <= 6 || rem[1] <= 6 || rem[2] <= 6) {
//	for (int i = 0; i < 13; ++i) {
//	    if (my_shape[i] == 4) {
//		Hand bomb_hand = Hand(i, 4, 1);
//		//bomb_hand.show();
//		if (check_valid(bomb_hand, prev)) {
//		    redo(bomb_hand);
//		    double val = evaluate();
//		    if (val > max_val) {
//			max_val = val;
//			max_hand = bomb_hand;
//		    }
//		    undo(bomb_hand);
//		}
//	    }
//	}
//	if (my_shape[13] == 1 && my_shape[14] == 1) {
//	    Hand bomb_hand = Hand(13, 1, 2);
//	    //bomb_hand.show();
//	    if (check_valid(bomb_hand, prev)) {
//		redo(bomb_hand);
//		double val = evaluate();
//		if (val > max_val) {
//		    max_val = val;
//		    max_hand = bomb_hand;
//		}
//		undo(bomb_hand);
//	    }
//	}
//    }
//
//    /** TODO: Search 4+1+1 or 4+2+2 **/
//
//    /** Search Pass (May be used for cooperation) **/
//    //double val = evaluate();
//    //if (val > max_val) {
//    //    max_val = val;
//    //    max_hand = Hand();
//    //}
//
//    //cout << "final result :" << endl;
//    //max_hand.show();
//    return hand_to_card(max_hand);
//}


vector<int> mc_play(Hand prev) {
    auto max_hand = Hand();
    double max_win_rate = -10000.;
    /** Search Strings, 3+2 and 3+1 **/
    for (auto left = 0; left < 13; ++left) {
	for (auto width = 1; width < 4; ++width) {
	    auto length = get_length_with_width(left, width);
	    auto min_length = 7-2*width;
	    for (auto l = min_length; l <= length; ++l) {
		Hand string_hand = Hand(left, width, l);
		// Add carriables for 3-string or pure 3
		if (width == 3) {
		    vector<vector<int>> carriables;
		    for (auto carry_width = 1; carry_width < width; ++carry_width) {
			vector<int> carriable;
			for (auto i = 0; i < 15; ++i) {
			    if (i >= left && i < left+l) {
				continue;
			    }
			    if (my_shape[i] < carry_width) {
				continue;
			    }
			    carriable.push_back(i);
			}
			carriables.push_back(carriable);
		    }

		    for(auto carry_width = 1; carry_width < width; ++carry_width) {
			auto carriable = carriables[carry_width-1];
			if (carriable.size() < length) {
			    continue;
			} 
			//string_hand.show();
			auto combinations = generate_combination(carriable.size(), l);
			for (auto combination : combinations) {
			    vector<int> carry;
			    for (auto index : combination) {
				for (int j = 0; j < carry_width; ++j) {
				    carry.push_back(carriable[index]);
				}
			    }
			    string_hand.set_carry(carry);
			    //string_hand.show();
			    /** Start main evaluation area **/
			    if (check_valid(string_hand, prev)) {
				redo(string_hand);
				// Monto Carlo To End
				int win_num = 0;
				for (int i = 0; i < mc_game_number; ++i) {
				    mc_init();
				    int win_pos = mc_run(string_hand);
				    if (win_pos == 0 && my_pos == 0) ++win_num;
				    if (win_pos != 0 && my_pos != 0) ++win_num;
				}
				double win_rate = (double) win_num / (double) mc_game_number;
				//string_hand.show();
				//cout << "win rate " << win_rate << endl;
				if (win_rate > max_win_rate) {
				    max_hand = string_hand;
				    max_win_rate = win_rate;
				}
				undo(string_hand);
			    }
			    /** End main evaluation area **/
			    string_hand.clear_carry();
			}
		    }
		}

		if (!check_valid(string_hand, prev)) {
		    continue;
		}
		//string_hand.show();
		redo(string_hand);
		// Monto Carlo To End
		int win_num = 0;
		for (int i = 0; i < mc_game_number; ++i) {
		    mc_init();
		    int win_pos = mc_run(string_hand);
		    if (win_pos == 0 && my_pos == 0) ++win_num;
		    if (win_pos != 0 && my_pos != 0) ++win_num;
		}
		double win_rate = (double) win_num / (double) mc_game_number;
		//string_hand.show();
		//cout << "win rate " << win_rate << endl;
		if (win_rate > max_win_rate) {
		    max_hand = string_hand;
		    max_win_rate = win_rate;
		}
		undo(string_hand);
	    }
	}
    }

    /** Search pure 1 or 2  card hands **/
    for (int i = 0; i < 15; ++i) {
	for (int width = 1; width <= my_shape[i] && width <= 2; ++width) {
	    Hand pure_hand = Hand(i, width, 1);
	    //pure_hand.show();
	    if (check_valid(pure_hand, prev)) {
		redo(pure_hand);
		// Monto Carlo To End
		int win_num = 0;
		for (int i = 0; i < mc_game_number; ++i) {
		    mc_init();
		    int win_pos = mc_run(pure_hand);
		    if (win_pos == 0 && my_pos == 0) ++win_num;
		    if (win_pos != 0 && my_pos != 0) ++win_num;
		}
		double win_rate = (double) win_num / (double) mc_game_number;
		//pure_hand.show();
		//cout << "win rate " << win_rate << endl;
		if (win_rate > max_win_rate) {
		    max_hand = pure_hand;
		    max_win_rate = win_rate;
		}
		undo(pure_hand);
	    }
	}
    }

    /** Search Bomb **/
    if (rem[0] <= 6 || rem[1] <= 6 || rem[2] <= 6) {
	for (int i = 0; i < 13; ++i) {
	    if (my_shape[i] == 4) {
		Hand bomb_hand = Hand(i, 4, 1);
		//bomb_hand.show();
		if (check_valid(bomb_hand, prev)) {
		    redo(bomb_hand);
		    // Monto Carlo To End
		    int win_num = 0;
		    for (int i = 0; i < mc_game_number; ++i) {
			mc_init();
			int win_pos = mc_run(bomb_hand);
			if (win_pos == 0 && my_pos == 0) ++win_num;
			if (win_pos != 0 && my_pos != 0) ++win_num;
		    }
		    double win_rate = (double) win_num / (double) mc_game_number;
		    //bomb_hand.show();
		    //cout << "win rate " << win_rate << endl;
		    if (win_rate > max_win_rate) {
			max_hand = bomb_hand;
			max_win_rate = win_rate;
		    }
		    undo(bomb_hand);
		}
	    }
	}
	if (my_shape[13] == 1 && my_shape[14] == 1) {
	    Hand bomb_hand = Hand(13, 1, 2);
	    //bomb_hand.show();
	    if (check_valid(bomb_hand, prev)) {
		redo(bomb_hand);
		// Monto Carlo To End
		int win_num = 0;
		for (int i = 0; i < mc_game_number; ++i) {
		    mc_init();
		    int win_pos = mc_run(bomb_hand);
		    if (win_pos == 0 && my_pos == 0) ++win_num;
		    if (win_pos != 0 && my_pos != 0) ++win_num;
		}
		double win_rate = (double) win_num / (double) mc_game_number;
		//bomb_hand.show();
		//cout << "win rate " << win_rate << endl;
		if (win_rate > max_win_rate) {
		    max_hand = bomb_hand;
		    max_win_rate = win_rate;
		}
		undo(bomb_hand);
	    }
	}
    }

    /** TODO: Search 4+1+1 or 4+2+2 **/

    /** Search Pass (May be used for cooperation) **/
    // Monto Carlo To End
    if (!prev.is_pass()) {
	int win_num = 0;
	for (int i = 0; i < mc_game_number; ++i) {
	    mc_init();
	    int win_pos = mc_run(Hand());
	    if (win_pos == 0 && my_pos == 0) ++win_num;
	    if (win_pos != 0 && my_pos != 0) ++win_num;
	}
	double win_rate = (double) win_num / (double) mc_game_number;
	//cout << "pass" << endl;
	//cout << "win rate " << win_rate << endl;
	if (win_rate > max_win_rate) {
	    max_hand = Hand();
	    max_win_rate = win_rate;
	}
    }

    //cout << "final result :" << endl;
    //max_hand.show();
    return hand_to_card(max_hand);
}

void input() {
    string line;
    getline(cin, line);
    Json::Value input;
    Json::Reader reader;
    reader.parse(line, input);

    //cout << input << endl; // for debug

    // First request
    {
	auto first_request = input["requests"][0u]; // 
	auto own = first_request["own"];
	//auto dipai = first_request["public"];
	auto first_history = first_request["history"];
	for (unsigned i = 0; i < own.size(); i++) {
	    int card = own[i].asInt();
	    my_cards.insert(card);
	    --remain_shape[card_to_point(card)];
	}
	// Get my position
	if (first_history[0u].size() == 0) {
	    if (first_history[1].size() == 0) {
		my_pos = 0; // Dizhu
	    } else {
		my_pos = 1; // Xiajia
	    }
	} else {
	    my_pos = 2; // Menban
	}
    }

    int turn_number = input["requests"].size();

    for (int turn = 0; turn < turn_number; ++turn) {
	auto history = input["requests"][turn]["history"]; 

	// Recover Remain Shape and Remain Card Numbers
	for (int pos = 0; pos < 2; pos++) {
	    int player_position = (my_pos + pos + 1) % 3;
	    auto player_action = history[pos]; 
	    for (int i = 0; i < player_action.size(); i++) {
		int card = player_action[i].asInt(); 
		--remain_shape[card_to_point(card)];
	    }
	    rem[player_position] -= player_action.size();
	}

	if (turn != turn_number - 1) {
	    // Recover My Shape and My Card and My Card Numbers
	    auto my_action = input["responses"][turn]; 
	    for (int i = 0; i < my_action.size(); i++) {
		int card = my_action[i].asInt(); 
		my_cards.erase(card); 
	    }
	    rem[my_pos] -= my_action.size();
	} else {
	    // Record last hand
	    for (int pos = 1; pos >= 0; --pos) {
		if (history[pos].size() != 0) {
		    for (int i = 0; i < history[pos].size(); ++i) {
			last_hand_cards.insert(history[pos][i].asInt());
		    }
		    break;
		}
	    }
	}
    }

    my_shape = card_to_shape(my_cards);
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
    std::srand( unsigned (std::time(0) ) );
    init();
    input();
    //cout << my_pos << endl;
    auto last_hand_shape = card_to_shape(last_hand_cards);
    auto last_hand = shape_to_hand(last_hand_shape);
    /** Below for debug **/
    //for (int i  = 0; i < 15; ++i) {
    //    cout << my_shape[i] << ' ';
    //}
    //cout << endl;
    //for (int i  = 0; i < 15; ++i) {
    //    cout << remain_shape[i] << ' ';
    //}
    //cout << endl;
    //for (int i  = 0; i < 3; ++i) {
    //    cout << rem[i] << ' ';
    //}
    //cout << endl;
    //for (auto card : last_hand_cards) {
    //    cout << card << ' ';
    //} 
    //cout << endl;
    /** Above for debug **/
    if (rem[my_pos] > 15) {
	mc_game_number = 25;
    } else if (rem[my_pos] > 10) {
	mc_game_number = 50;
    } else if (rem[my_pos] > 5) {
	mc_game_number = 100;
    } else {
	mc_game_number = 200;
    }
    mc_game_number *= 2;
    if (!last_hand.is_pass()) {
	mc_game_number *= 4;
    }
    output(mc_play(last_hand));
}
