#include<iostream>
#include<vector>
#include<ctime>
#include<cstdlib>
#include<algorithm>
#include<fstream>

using namespace std;

int shape[3][15];
int origin[3][15];
int rem[3];
vector< vector<int> > history;

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



void output_shape() {
    vector<int> status;
    for (int j = 0; j < 3; ++j) {
	for (int i = 0; i < 15; ++i) {
	    status.push_back(shape[j][i]);
	}
    }
    for (int i = 0; i < 3; ++i) {
	status.push_back(rem[i]);
    }
    history.push_back(status);
}


struct Hand {
    int length;
    int point;
    Hand(int _l, int _p) {
	length = _l;
	point = _p;
    };
    Hand() {length = 0;}
};

int cards_to_number(int card) {
    return card < 52 ? card / 4 : card - 39;
}


void random_start() {
    history.clear();
    for (int i = 0; i < 3; ++i) {
	for (int j = 0; j < 15; ++j) {
	    shape[i][j] = 0;
	    origin[i][j] = 0;
	}
    }
    vector<int> cards;
    rem[0] = 20;
    rem[1] = 17;
    rem[2] = 17;
    for(int i = 0; i < 54; ++i) {
	cards.push_back(i);
    }
    random_shuffle(cards.begin(), cards.end());
    for(int i = 0; i < 20; ++i) {
	shape[0][cards_to_number(cards[i])]++;
	origin[0][cards_to_number(cards[i])]++;
    }
    for(int i = 20; i < 37; ++i) {
	shape[1][cards_to_number(cards[i])]++;
	origin[1][cards_to_number(cards[i])]++;
    }
    for(int i = 37; i < 54; ++i) {
	shape[2][cards_to_number(cards[i])]++;
	origin[2][cards_to_number(cards[i])]++;
    }
    output_shape();
}

/**
Hand get_shunzi(int pos) {
    int last_zero = -1;
    //cout << "try" << endl;
    for (int i = 0; i < 13; ++i) {
	if (shape[pos][i] == 0 || i == 12) {
	    if (i - 1 - last_zero >= 5) {
		for (int j = last_zero + 1; j < i; ++j) {
		    shape[pos][j] -= 1;
		}
		rem[pos] -= i - 1 - last_zero;
		return Hand(i - 1 - last_zero, last_zero + 1);
	    } else {
		last_zero = i;
	    }
	} 
    }
    return Hand();
}
**/

int check_shunzi(int start, int width, int pos) {
    //cout << "check shunzi " << start << " " << pos << endl;
    int right;
    for (right = start; right < 12; ++right) {
	if (shape[pos][right] < width) {
	    break;
	}
    }
    //cout << "s " << start << " | r " << right << endl;
    int distance = right - start + 1;
    if (width == 1 && distance < 5) {
	return 0;
    }
    if (width == 2 && distance < 3) {
	return 0;
    }
    if (width == 3 && distance < 2) {
	return 0;
    }
    return distance;
}

/**
Hand new_random_hand(int pos) {
    int shunzi = rand() % 4;
    if (shunzi != 1) {
	Hand hand = get_shunzi(pos);
	if (hand.length != 0) {
	    return hand;
	}
    }
    vector<int> non_zero;
    for (int i = 0; i < 15; ++i) {
	//std::cout << shape[pos][i] << ',';
	if (shape[pos][i] != 0) {
	    non_zero.push_back(i);
	}
    }
    //cout << " at " << pos << ' ' << rem[0] << ' ' << rem[1] << ' ' << rem[2];
    //cout << " nz " << non_zero.size() << endl;
    int place = rand() % non_zero.size();
    int l = shape[pos][non_zero[place]];
    shape[pos][non_zero[place]] = 0;
    rem[pos] -= l;
    return Hand(l, non_zero[place]);
}
**/

/**
Hand follow_shunzi(int pos, Hand prev) {
    int last_zero = prev.point;
    for (int i = prev.point + 1; i < 13; ++i) {
	if (i - 1 - last_zero == prev.length) {
	    for (int j = last_zero + 1; j < i; ++j) {
		shape[pos][j] -= 1;
	    }
	    rem[pos] -= prev.length;
	    return Hand(i - 1 - last_zero, last_zero + 1);
	} else if (shape[pos][i] == 0) {
	    last_zero = i;
	} 
    }
    return Hand();
}
**/
    
Hand bomb(int pos) {
    if (rem[0] >= 6 && rem[1] >= 6 && rem[2] >= 6) {
	return Hand();
    }
    for (int i = 0; i < 12; ++i) {
	if (shape[pos][i] == 4) {
	    shape[pos][i] -= 4;
	    rem[pos] -= 4;
	    //cout << "BOOM" << std::endl;
	    return Hand(-1, i);
	}
    }
    if (shape[pos][13] == 1 && shape[pos][14] == 1) {
	//cout << "BOOM" << std::endl;
	shape[pos][13] = 0;
	shape[pos][14] = 0;
	rem[pos] -= 2;
	return Hand(-1, -1);
    } 
    return Hand();
}

/**
Hand follow_smallest(int pos, Hand prev_hand) {
    if (prev_hand.length == -1) {
	return Hand();
    }
    if (prev_hand.length <= 3) {
	if (pos == 2) {
	    for (int i = 14; i >= prev_hand.point + 1; --i) {
		if (shape[pos][i] >= prev_hand.length) {
		    shape[pos][i] -= prev_hand.length;
		    rem[pos] -= prev_hand.length;
		    return Hand(prev_hand.length, i);
		}
	    }
	} else {
	    for (int i = prev_hand.point + 1; i < 15; ++i) {
		if (shape[pos][i] >= prev_hand.length) {
		    shape[pos][i] -= prev_hand.length;
		    rem[pos] -= prev_hand.length;
		    return Hand(prev_hand.length, i);
		}
	    }
	}
	return bomb(pos);
    } else {
	auto hand = follow_shunzi(pos, prev_hand);
	if (hand.length == 0) {
	    return bomb(pos);
	}
    }
}
**/

bool check_valid(Hand hand, Hand prev) {
    if (prev.length == 0) {
	return true;
    } else if (prev.length != hand.length) {
	return false;
    } else {
	return prev.point < hand.point;
    }
    return false;
}

void redo(Hand hand, int pos) {
    if (hand.length == -1) {
	if (hand.point == -1) {
	    shape[pos][13] = 0;
	    shape[pos][14] = 0;
	    rem[pos] -= 2;
	} else {
	    shape[pos][hand.point] -= 4;
	    rem[pos] -= 4;
	}
	return;
    } else if (hand.length == 0) {
	return;
    } else if (hand.length <= 4) {
	shape[pos][hand.point] -= hand.length;
	rem[pos] -= hand.length;
    } else if (hand.length < 20) {
	for (int i = 0; i < hand.length; ++i) {
	    shape[pos][hand.point + i]--;
	}
	rem[pos] -= hand.length;
    } else if (hand.length < 30) {
	for (int i = 0; i < hand.length-20; ++i) {
	    shape[pos][hand.point + i] -= 2;
	}
	rem[pos] -= 2*(hand.length-20);
    } else if (hand.length < 40) {
	for (int i = 0; i < hand.length-30; ++i) {
	    shape[pos][hand.point + i] -= 3;
	}
	rem[pos] -= 3*(hand.length-30);
    }
    return;
}

void undo(Hand hand, int pos) {
    if (hand.length == -1) {
	if (hand.point == -1) {
	    shape[pos][13] = 1;
	    shape[pos][14] = 1;
	    rem[pos] += 2;
	} else {
	    shape[pos][hand.point] += 4;
	    rem[pos] += 4;
	}
	return;
    } else if (hand.length == 0) {
	return;
    } else if (hand.length <= 4) {
	shape[pos][hand.point] += hand.length;
	rem[pos] += hand.length;
    } else if (hand.length < 20) {
	for (int i = 0; i < hand.length; ++i) {
	    shape[pos][hand.point + i]++;
	}
	rem[pos] += hand.length;
    } else if (hand.length < 30) {
	for (int i = 0; i < hand.length-20; ++i) {
	    shape[pos][hand.point + i] += 2;
	}
	rem[pos] += 2*(hand.length-20);
    } else if (hand.length < 40) {
	for (int i = 0; i < hand.length-30; ++i) {
	    shape[pos][hand.point + i] += 3;
	}
	rem[pos] += 3*(hand.length-30);
    }
    return;
}


double evaluate(int pos) {
    vector<double> features;
    features.push_back(1.0);
    for (int i = 1; i < 15; ++i) {
	features.push_back(double(shape[pos][i]));
    }
    for (int i = 0; i < 3; ++i) {
	features.push_back(double(rem[i]));
    }
    int last_zero = -1;
    int length = 0;
    int length2 = 0;
    int length3 = 0;
    for (int j = 0; j < 13; ++j) {
	if (shape[pos][j] == 0 || j == 12) {
	    if (j - 1 - last_zero >= 5) {
		if (length < j - 1 - last_zero) {
		    length = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }
    last_zero = -1;
    for (int j = 0; j < 13; ++j) {
	if (shape[pos][j] < 2 || j == 12) {
	    if (j - 1 - last_zero >= 3) {
		if (length2 < j - 1 - last_zero) {
		    length2 = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }
    last_zero = -1;
    for (int j = 0; j < 13; ++j) {
	if (shape[pos][j] < 3 || j == 12) {
	    if (j - 1 - last_zero >= 2) {
		if (length3 < j - 1 - last_zero) {
		    length3 = j - 1 - last_zero;
		}
	    }
	    last_zero = j;
	} 
    }
    features.push_back(double(length));
    features.push_back(double(length2));
    features.push_back(double(length3));
    int non_zero_num = 0;
    int has_bomb = 0;
    for (int j = 0; j < 15; ++j) {
	if (shape[pos][j] != 0) {
	    ++non_zero_num;
	}
	if (shape[pos][j] == 4) {
	    ++has_bomb;
	}
    }
    if (shape[pos][14] == 1 && shape[pos][13] == 1) {
	++has_bomb;
    }

    features.push_back(double(non_zero_num));
    features.push_back(double(has_bomb));
    cout << "features: ";
    for (auto feature : features) {
        cout << feature << ',';
    }
    cout << endl;
    double value = 0.;
    for (int i = 0; i < features.size(); ++i) {
	value += features[i] * weight[pos][i];
    } 
    return value;
}

bool break_bomb(Hand hand, int pos) {
    if (rem[pos] <= 6) {
	return false;
    }
    //std::cout << "check break bomb " << hand.point << 'x' << hand.length << " at " << pos << endl;
    if (hand.length == 0) {
	return false;
    } else if (hand.length == 1) {
	if (hand.point == 13 || hand.point == 14) {
	    return (shape[pos][13] == 1 && shape[pos][14] == 1);
	} else {
	    return shape[pos][hand.point] == 4;
	}
    } else if (hand.length <= 4) {
	return shape[pos][hand.point] == 4;
    } else if (hand.length < 40) {
	for (int i = hand.point; i < hand.length+hand.point; ++i) {
	    if (shape[pos][i] == 4) {
		return true;
	    }
	}
	return false;
    }
    return false;
}

Hand max_hand(int pos, Hand prev) {
    if (prev.length == -1) {
	return Hand();
    }
    Hand max_hand = Hand();
    double max_val = -10000.;
    for (int start = 0; start < 7; ++start) {
	for (int width = 1; width < 4; ++width) {
	    int shunzi_length = check_shunzi(start, width, pos);
	    int base = 5;
	    if (width == 2 || width == 3) {
		base = 5 - width;
	    }
	    for (int i = base; i < shunzi_length; ++i) {
		auto hand = width == 1 ? Hand(i, start) : Hand(width*10 + i, start);
		//cout << hand.length << std::endl;
		if (!check_valid(hand, prev) || break_bomb(hand, pos)) {
		    continue;
		}
		redo(hand, pos);
		//cout << "after redo : " << rem[pos] << endl;
		double val = evaluate(pos);
		cout << pos << ':' << hand.point << 'x' << hand.length << '=' << val << endl;
		if (val > max_val) {
		    max_hand = hand;
		    max_val = val;
		}
		undo(hand, pos);
		//cout << hand.length << std::endl;
		//cout << "after undo : " << rem[pos] << endl;
	    }
	}
    }
    vector<int> non_zero;
    for (int i = 0; i < 15; ++i) {
	if (shape[pos][i] != 0) {
	    non_zero.push_back(i);
	}
    }
    //for (int i = 0; i < non_zero.size(); ++i) {
	//cout << non_zero[i] << "*" << shape[pos][non_zero[i]] << " ";
    //}
    //cout << endl;
    for (int i = 0; i < non_zero.size(); ++i) {
	for (int j = 1; j <= shape[pos][non_zero[i]] && j <= 3; ++j) {
	    Hand temp_hand = Hand(j, non_zero[i]);
	    if (!check_valid(temp_hand, prev) || break_bomb(temp_hand, pos)) {
		continue;
	    }
	    shape[pos][non_zero[i]] -= j;
	    rem[pos] -= j;
	    double val = evaluate(pos);
	    //cout << pos << " try " << temp_hand.point << "x" << temp_hand.length << "=" << val << endl;
	    cout << pos << ':' << temp_hand.point << 'x' << temp_hand.length << '=' << val << endl;
	    if (val > max_val) {
		max_hand = temp_hand;
		max_val = val;
	    }
	    shape[pos][non_zero[i]] += j;
	    rem[pos] += j;
	    //cout << pos << ":" << i << " and " << j << endl;
	}
    }
    //cout << pos << endl;
    if (prev.length != 0 && max_hand.length == 0) {
	return bomb(pos);
    }
    if (max_hand.length != 0) {
	redo(max_hand, pos);
    }
    //cout << max_hand.point << " go " << max_hand.length << endl;
    return max_hand;
}


Hand one_hand(int position, Hand prev) {
    return max_hand(position, prev);
    //if (prev.length == 0) {
    //    return new_random_hand(position);
    //} else {
    //    return follow_smallest(position, prev);
    //}
}



void output_all(int pos) {
    ofstream dizhu_file;
    ofstream xiajia_file;
    ofstream menban_file;
    dizhu_file.open("dizhu.dat", std::ios_base::app);
    xiajia_file.open("xiajia.dat", std::ios_base::app);
    menban_file.open("menban.dat", std::ios_base::app);
    for (int i = 0; i < history.size(); ++i) {
	int my_pos = (i) % 3;
	bool win = false;
	if (pos == 0 && my_pos == 0) {
	    win = true;
	} else if (pos != 0 && my_pos != 0) {
	    win = true;
	}
	vector<int> output;
	//for (int j = 0; j < 15; ++j) {
	//    output.push_back(history[i-1][my_pos*15+j]);
	//}
	for (int j = 0; j < 15; ++j) {
	    output.push_back(history[i][my_pos*15+j]);
	}
	output.push_back(history[i][45]);
	output.push_back(history[i][46]);
	output.push_back(history[i][47]);
	int last_zero = -1;
	int length = 0;
	int length2 = 0;
	int length3 = 0;
	for (int j = 0; j < 13; ++j) {
	    if (output[j] == 0 || j == 12) {
		if (j - 1 - last_zero >= 5) {
		    //cout << pos << ' ' << "j = " << j << ' ' << last_zero << endl;
		    if (length < j - 1 - last_zero) {
			length = j - 1 - last_zero;
		    }
		}
		last_zero = j;
	    } 
	}
	last_zero = -1;
	for (int j = 0; j < 13; ++j) {
	    if (output[j] < 2 || j == 12) {
		if (j - 1 - last_zero >= 3) {
		    if (length2 < j - 1 - last_zero) {
			length2 = j - 1 - last_zero;
		    }
		}
		last_zero = j;
	    } 
	}
	last_zero = -1;
	for (int j = 0; j < 13; ++j) {
	    if (output[j] < 3 || j == 12) {
		if (j - 1 - last_zero >= 2) {
		    if (length3 < j - 1 - last_zero) {
			length3 = j - 1 - last_zero;
		    }
		}
		last_zero = j;
	    } 
	}
	output.push_back(length);
	output.push_back(length2);
	output.push_back(length3);
	int non_zero_num = 0;
	int has_bomb = 0;
	for (int j = 0; j < 15; ++j) {
	    if (output[j] != 0) {
		++non_zero_num;
	    }
	    if (output[j] == 4) {
		++has_bomb;
	    }
	}
	if (output[14] == 1 && output[13] == 1) {
	    ++has_bomb;
	}

	output.push_back(non_zero_num);
	output.push_back(has_bomb);

	//int prev_pos = (my_pos + 2) % 3;
	//int next_pos = (my_pos + 1) % 3;
	//for (int j = 0; j < 15; ++j) {
	//    output.push_back(origin[prev_pos][j] - history[i][prev_pos*15+j]);
	//}
	//for (int j = 0; j < 15; ++j) {
	//    output.push_back(origin[next_pos][j] - history[i][next_pos*15+j]);
	//}
	//

	if (win) {
	    output.push_back(1);
	} else {
	    output.push_back(0);
	}
	for (auto result : output) {
	    if (my_pos == 0) {
		dizhu_file << result << " ";
	    } else if (my_pos == 1) {
		xiajia_file << result << " ";
	    } else {
		menban_file << result << " ";
	    }
	}
	if (my_pos == 0) {
	    dizhu_file << endl;
	} else if (my_pos == 1) {
	    xiajia_file << endl;
	} else {
	    menban_file << endl;
	}
    }
    dizhu_file.close();
    menban_file.close();
    xiajia_file.close();
}

int main() {
    std::srand ( unsigned ( std::time(0) ) );
    for (int k = 0; k < 1; ++k) {
    cout << k << endl;
    random_start();
    Hand prev_hand;
    int position = 0;
    int pass = 0;
    while(rem[0] != 0 && rem[1] != 0 && rem[2] != 0) {
	Hand hand = one_hand(position, prev_hand);
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
	//cout << hand.point << " x " << hand.length << endl;
	//cout << rem[0] << ' ' << rem[1] << ' ' << rem[2] << endl;
	output_shape();
    }
    int vic_pos;
    if(rem[0] == 0) {
	vic_pos = 0;
    } else if (rem[1] == 0) {
	vic_pos = 1;
    } else if (rem[2] == 0) {
	vic_pos = 2;
    }
    output_all(vic_pos);
    }
}

    
