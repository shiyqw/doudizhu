#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <functional>
#include <ctime>
#include <cstdlib>
#include <list>

#include "jsoncpp/json.h"

using namespace std;

/*****************************************************************************/
// Global Data
/*****************************************************************************/
int MC_GAME_NUMBERS = 5000;
double ELAPSED_SECS = -1.;
set<int> my_cards;
set<int> last_hand_cards;
set<int> public_cards;
vector<int> last_hand_shape;
vector<int> my_shape = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int remain_shape[15] = {4,4,4,4,4,4,4,4,4,4,4,4,4,1,1};
int rem[3] = {20,17,17};
int mc_rem[3];
int mc_shape[3][15];
int my_pos;
int last_hand_pos = -1;
int score[3];

/*****************************************************************************/
// Data Structure for Hand Type
/*****************************************************************************/
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

  //void show() {
  //    stringstream buffer;
  //    buffer << point << ":" << width << "x" << length;
  //    for (auto card : carry) {
  //        buffer << " " << card;
  //    }
  //    cout << setw(20) << left << buffer.str();
  //}

};

Hand last_hand;

bool my_comp(const pair<Hand, double> & a, const pair<Hand, double> & b) {
  return a.second < b.second;
}

bool check_valid(Hand hand, Hand prev) {
  // special cases
  if (prev.is_pass()) {
    if (hand.is_pass()) {
      return false;
    } else {
      return true;
    }
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

/*****************************************************************************/
// Weights and Calculations for Game Evaluation
/*****************************************************************************/
double weight[3][23] = { {
  2.4959,
  0.044437,
  0.076519,
  0.056397,
  0.048103,
  0.028913,
  0.017672,
  -0.020173,
  -0.0042281,
  0.016276,
  0.021673,
  0.09065,
  0.21604,
  0.67413,
  0.69404,
  0.16069,
  0.047595,
  0.25299,
  -0.34382,
  0.15859,
  0.17257,
  0.057833,
  0.071089
},{
  1.6165,
  -0.028985,
  -0.016422,
  -0.021543,
  -0.031163,
  -0.025262,
  -0.028609,
  -0.064754,
  -0.038231,
  -0.0012782,
  0.022632,
  0.095732,
  0.21099,
  0.42297,
  0.4918,
  0.053856,
  -0.013217,
  0.053965,
  -0.15296,
  0.11153,
  0.12702,
  0.028402,
  0.041108
},{
  2.124,
  -0.046211,
  -0.043883,
  -0.044154,
  -0.050309,
  -0.055101,
  -0.024143,
  -0.029271,
  -0.015032,
  0.0034946,
  0.0060442,
  0.016112,
  0.1064,
  0.22569,
  0.20185,
  0.064005,
  0.010855,
  -0.030842,
  -0.16538,
  0.26596,
  0.0036069,
  0.010204,
  0.036842
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

  // Get Control Number
  int control1 = 0;
  int control2 = 0;
  int control3 = 0;
  int j = 14;
  for (; j >= 0; --j) {
    if (mc_shape[(pos+1)%3][j] > 0) {
      break;
    }
    if (mc_shape[(pos+2)%3][j] > 0) {
      break;
    }
    control1 += mc_shape[pos][j];
  }

  for (j = 14; j >= 0; --j) {
    if (mc_shape[(pos+2)%3][j] + mc_shape[(pos+1)%3][j] >= 2) {
      break;
    }
    control2 += mc_shape[pos][j] >= 2 ? 2 : 0;
  }

  for (j = 14; j >= 0; --j) {
    if (mc_shape[(pos+2)%3][j] + mc_shape[(pos+1)%3][j] >= 3) {
      break;
    }
    control3 += mc_shape[pos][j] >= 3 ? 3 : 0;
  }

  // Get All features
  vector<double> features;
  features.push_back(1.0);
  for (int i = 1; i < 15; ++i) {
    features.push_back(double(mc_shape[pos][i]));
  }
  features.push_back(double(single_string_length));
  features.push_back(double(double_string_length));
  features.push_back(double(triple_string_length));
  features.push_back(double(non_zero_num));
  features.push_back(double(has_bomb));
  features.push_back(control1);
  features.push_back(control2);
  features.push_back(control3);

  double value = 0.;
  for (int i = 0; i < features.size(); ++i) {
    value += features[i] * weight[pos][i];
  }

  /** output features (for debug) **/
  //cout << "features: ";
  //for (auto feature : features) {
  //    cout << feature << ',';
  //}
  /** end output features **/
  //cout << " v:" << value << endl;

  return value;
}

/*****************************************************************************/
// Type Transformation between Hand:struct, Shape:vector, and Cards:set
/*****************************************************************************/
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

/*****************************************************************************/
// Helper Function for Generating Carriable Cards
/*****************************************************************************/
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

/*****************************************************************************/
// Methods for Game Processing and Backtracking in each Monto Carlo Game
/*****************************************************************************/
void mc_calc_detail(int pos, Hand hand) {
  if (hand.is_pass()) {
    return;
  }
  if (hand.width == 1 && hand.length == 1) {
    score[pos] += 1;
  }
  if (hand.width == 2 && hand.length == 1) {
    score[pos] += 2;
  }
  if (hand.width == 3 && hand.length == 1) {
    score[pos] += 4;
  }
  if (hand.width == 1 && hand.length > 1) {
    score[pos] += 6;
  }
  if (hand.width == 2 && hand.length > 1) {
    score[pos] += 6;
  }
  if (hand.width == 3 && hand.length > 1) {
    score[pos] += 8;
  }
  if (hand.width == 4 && !hand.carry.empty()) {
    if (hand.length == 1) {
      score[pos] += 8;
    } else {
        score[pos] += 10;
    }
  }
  if (hand.is_bomb()) {
    if (hand.width == 4) {
      score[pos] += 10;
    } else {
      score[pos] += 16;
    }
  }

}

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

vector<int> mc_shuffle() {
  vector<int> cards;
  if (my_pos != 0) {
    for (auto card : public_cards) {
      --remain_shape[card_to_point(card)];
    }
  }
  for (int i = 0; i < 15; ++i) {
    for (int j = 0; j < remain_shape[i]; ++j) {
      cards.push_back(i);
    }
  }
  random_shuffle(cards.begin(), cards.end());
  if (my_pos == 1) {
    for (auto card : public_cards) {
      cards.push_back(card_to_point(card));
    }
  } else if (my_pos == 2) {
    for (auto card : public_cards) {
      cards.insert(cards.begin(), card_to_point(card));
    }
  }
  if (my_pos != 0) {
    for (auto card : public_cards) {
      ++remain_shape[card_to_point(card)];
    }
  }
  return cards;
}


void mc_init(vector<int> cards) {
  for (int i = 0; i < 3; ++i) {
    mc_rem[i] = rem[i];
    score[i] = 0;
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
  for (int i = 0; i < rem[(my_pos+1)%3]; ++i) {
    mc_shape[(my_pos+1)%3][cards[i]]++;
  }
  for (int i = 0; i < rem[(my_pos+2)%3]; ++i) {
    mc_shape[(my_pos+2)%3][cards[i+rem[(my_pos+1)%3]]]++;
  }
}

int mc_get_length_with_width(int pos, int left, int width) {
  int right_limit = 12;
  if (left == 13) {
    right_limit = 13;
  }
  for (int i = left; i <= right_limit; ++i) {
    if (mc_shape[pos][i] < width || i == right_limit) {
      return (i-left);
    }
  }
  return 0;
}

/*****************************************************************************/
// Methods for Hands Redoing and Undoing for Searching Best Hand
/*****************************************************************************/
int get_length_with_width(int left, int width) {
  int right_limit = 12;
  if (left == 13) {
    right_limit = 13;
  }
  for (int i = left; i <= right_limit; ++i) {
    if (my_shape[i] < width || i == right_limit) {
      return (i-left);
    }
  }
  return 0;
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


/*****************************************************************************/
// One Morte Carlo Step for One Player
/*****************************************************************************/
Hand mc_step(int pos, Hand prev, int random_number) {
  auto max_hand = Hand();
  double max_val = -10000.;
  list<pair<Hand, double>> hands;
  /** Search Strings, 3+2 and 3+1 **/
  if (prev.length == 0 || prev.length * prev.width >= 3) {
    for (auto left = 0; left < 13; ++left) {
      for (auto width = 1; width <= 4; ++width) {
        auto length = mc_get_length_with_width(pos, left, width);
        auto min_length = 7-2*width;
        if (width == 4) {
          min_length = 1;
        }
        for (auto l = min_length; l <= length; ++l) {
          Hand string_hand = Hand(left, width, l);
          // Add carriables for 3-string or pure 3
          if (width >= 3) {
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

            for (auto carry_width = 1; carry_width < width; ++carry_width) {
              auto carriable = carriables[carry_width-1];
              if (carriable.size() < (width-2)*l) {
                continue;
              }
              auto combinations = generate_combination(carriable.size(), (width-2)*l);
              for (auto combination : combinations) {
                vector<int> carry;
                for (auto index : combination) {
                  for (int j = 0; j < carry_width; ++j) {
                    carry.push_back(carriable[index]);
                  }
                }
                string_hand.set_carry(carry);
                /** Start main evaluation area **/
                if (check_valid(string_hand, prev)) {
                  mc_redo(pos, string_hand);
                  double val = mc_evaluate(pos);
                  hands.push_back(make_pair(string_hand, val));
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
          mc_redo(pos, string_hand);
          // Check pure string
          auto val = mc_evaluate(pos);
          hands.push_back(make_pair(string_hand, val));
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
        if (check_valid(pure_hand, prev)) {
          mc_redo(pos, pure_hand);
          double val = mc_evaluate(pos);
          hands.push_back(make_pair(pure_hand, val));
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
        if (check_valid(bomb_hand, prev)) {
          mc_redo(pos, bomb_hand);
          double val = mc_evaluate(pos);
          hands.push_back(make_pair(bomb_hand, val));
          mc_undo(pos, bomb_hand);
        }
      }
    }
    if (mc_shape[pos][13] == 1 && mc_shape[pos][14] == 1) {
      Hand bomb_hand = Hand(13, 1, 2);
      if (check_valid(bomb_hand, prev)) {
        mc_redo(pos, bomb_hand);
        double val = mc_evaluate(pos);
        hands.push_back(make_pair(bomb_hand, val));
        mc_undo(pos, bomb_hand);
      }
    }
  }

  /** Search Pass (May be used for cooperation) **/
  Hand pass_hand = Hand();
  if (check_valid(pass_hand, prev)) {
    mc_redo(pos, pass_hand);
    double val = mc_evaluate(pos);
    hands.push_back(make_pair(pass_hand, val));
    mc_undo(pos, pass_hand);
  }

  /** Randomized **/
  random_number = min(random_number, (int) hands.size());
  for (int i = 0; i < random_number-1; ++i) {
    hands.erase(max_element(hands.begin(), hands.end(), my_comp));
  }

  if (!hands.empty()) {
    max_hand = max_element(hands.begin(), hands.end(), my_comp)->first;
  }
  mc_calc_detail(pos, max_hand);
  mc_redo(pos, max_hand);
  return max_hand;
}

/*****************************************************************************/
// Monte Carlo Process (One Game)
/*****************************************************************************/
int mc_run(Hand prev_hand) {

  //cout << endl;
  //cout << "start run" << endl;
  //prev_hand.show();
  //cout << endl;
  //for (int i = 0; i < 3; ++i) {
  //    for (int j = 0; j < 15; ++j) {
  //        cout << mc_shape[i][j] << " ";
  //    }
  //    cout << endl;
  //}
  //cout << "********************" << endl;

  int position = (my_pos + 1) % 3;
  int pass = 0;
  while(mc_rem[0] != 0 && mc_rem[1] != 0 && mc_rem[2] != 0) {
    int random_number = 1;

    /** Below should be toggled if we want add random factors **/
    //if (rem[0] + rem[1] + rem[2] <= 25) {
    //    random_number = 3;
    //}
    //if (rem[0] + rem[1] + rem[2] <= 15) {
    //    random_number = 5;
    //}

    Hand hand = mc_step(position, prev_hand, random_number);
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
    //      cout << mc_shape[i][j] << " ";
    //    }
    //    cout << endl;
    //}
    //cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

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

/*****************************************************************************/
// Search Process: Start Monte Carlo Process for Each Hand and Choose the Best
/*****************************************************************************/
vector<int> mc_play(Hand prev) {
  clock_t start_time = clock();
  auto max_hand = Hand();
  list<pair<Hand, int>> hands;
  double max_win_rate = -10000.;

  /** Search Strings, 3+2 and 3+1 **/
  for (auto left = 0; left < 13; ++left) {
    for (auto width = 1; width <= 4; ++width) {
      auto length = get_length_with_width(left, width);
      auto min_length = 7-2*width;
      if (width == 4) {
        min_length = 1;
      }
      for (auto l = min_length; l <= length; ++l) {
        Hand string_hand = Hand(left, width, l);
        // Add carriables for 3-string or pure 3
        if (width >= 3) {
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

          for (auto carry_width = 1; carry_width < width; ++carry_width) {
            auto carriable = carriables[carry_width-1];
            if (carriable.size() < (width-2)*l) {
                continue;
            }
            //string_hand.show();
            auto combinations = generate_combination(carriable.size(), (width-2)*l);
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
                hands.push_back(make_pair(string_hand, 0));
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
        hands.push_back(make_pair(string_hand, 0));
      }
    }
  }

  /** Search pure 1 or 2  card hands **/
  for (int i = 0; i < 15; ++i) {
    for (int width = 1; width <= my_shape[i] && width <= 2; ++width) {
      Hand pure_hand = Hand(i, width, 1);
      //pure_hand.show();
      if (check_valid(pure_hand, prev)) {
        hands.push_back(make_pair(pure_hand, 0));
      }
    }
  }

  /** Search Bomb **/
  if ((rem[0] <= 6 || rem[1] <= 6 || rem[2] <= 6)
        && my_pos+last_hand_pos != 3
        && prev.length != 0) {
    for (int i = 0; i < 13; ++i) {
      if (my_shape[i] == 4) {
        Hand bomb_hand = Hand(i, 4, 1);
        //bomb_hand.show();
        if (check_valid(bomb_hand, prev)) {
          hands.push_back(make_pair(bomb_hand, 0));
        }
      }
    }
    if (my_shape[13] == 1 && my_shape[14] == 1) {
      Hand bomb_hand = Hand(13, 1, 2);
      //bomb_hand.show();
      if (check_valid(bomb_hand, prev)) {
        hands.push_back(make_pair(bomb_hand, 0));
      }
    }
  }

  /** Search Pass (May be used for cooperation) **/
  // Monto Carlo To End
  if (!prev.is_pass()) {
    hands.push_back(make_pair(Hand(), 0));
  }

  if (hands.size() == 0) {
    max_hand = Hand();
  } else if (hands.size() == 1) {
    max_hand = hands.front().first;
  } else {

    // Optimizaion
    int k = hands.size();
    vector<int> n;
    double logk = 0.5;
    for (int i = 2; i <= k; ++i) {
        logk += 1.0 / (double) i;
    }
    n.push_back(0);
    for (int i = 1; i < k; ++i) {
      int nk = (int) ceil(1.0 / logk * ((double) (MC_GAME_NUMBERS-k)) / ((double) (k+1-i)));
      n.push_back(nk);
    }

    for (int i = 1; i <= k-1; ++i) {
      int game_number = n[i]-n[i-1];
      for (int j = 0; j < game_number; ++j) {
        auto cards = mc_shuffle();
        for (auto it = hands.begin(); it != hands.end(); ++it) {
          auto hand = it->first;
          redo(hand);
          mc_init(cards);
          mc_calc_detail(my_pos, hand);
          int win_pos = mc_run(hand);
          if (my_pos == 0) {
            int delta = 2*score[0] - (score[1]+score[2]);
            it->second += delta;
            if (win_pos == 0) {
              it->second += 400;
            } else {
              it->second -= 400;
            }
          } else if (my_pos != 0) {
            int delta = (score[1]+score[2]) - 2*score[0];
            it->second += delta;
            if (win_pos == 0) {
              it->second -= 400;
            } else {
              it->second += 400;
            }
          }
          undo(hand);
        }
        clock_t current_time = clock();
        ELAPSED_SECS = double (current_time - start_time) / CLOCKS_PER_SEC;
        if (ELAPSED_SECS > 0.9) {
          int max_win = -0x7FFFFFFF;
          list<pair<Hand, int>>::iterator best_hand;
          for (auto it = hands.begin(); it != hands.end(); ++it) {
            if (it->second > max_win) {
              best_hand = it;
              max_win = it->second;
            }
          }

          max_hand = best_hand->first;
          return hand_to_card(max_hand);
        }
      }
      int min_win = 0x7FFFFFFF;
      list<pair<Hand, int>>::iterator worst_hand, it;

      //for (auto hand_with_win : hands) {
      //    hand_with_win.first.show();
      //    cout << " win num " << hand_with_win.second / 200. / (double) n[i] << endl;
      //}
      //cout << "-------------------------------" << endl;

      for (auto it = hands.begin(); it != hands.end(); ++it) {
        if (it->second < min_win) {
          worst_hand = it;
          min_win = it->second;
        }
      }
      hands.erase(worst_hand);

      //cout << "used seconds:" << ELAPSED_SECS << endl;

    }
    max_hand = hands.front().first;
  }

  return hand_to_card(max_hand);
}

/*****************************************************************************/
// JSON I/O
/*****************************************************************************/
void input() {
  string line;
  getline(cin, line);
  Json::Value input;
  Json::Reader reader;
  reader.parse(line, input);

  // First request
  {
    auto first_request = input["requests"][0u]; 
    auto own = first_request["own"];
    auto dipai = first_request["publiccard"];
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

    for (int i = 0; i < 3; ++i) {
      public_cards.insert(dipai[i].asInt());
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
        if (public_cards.count(card) > 0) {
          public_cards.erase(card);
        }
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
          last_hand_pos = (my_pos + 1 + pos) % 3;
          break;
        }
      }
    }
  }

  my_shape = card_to_shape(my_cards);
}

void output(vector<int> cards) {
  Json::Value result, response(Json::arrayValue);
  for (auto card : cards) {
    response.append(card);
  }
  result["response"] = response;
  result["debug"] = ELAPSED_SECS;
  Json::FastWriter writer;
  cout << writer.write(result) << endl;
}

/*****************************************************************************/
// Main Program: Get Input, Get Recent Game Status, Start Search and Output
/*****************************************************************************/
int main() {
  std::srand(unsigned(std::time(0)));
  input();
  auto last_hand_shape = card_to_shape(last_hand_cards);
  auto last_hand = shape_to_hand(last_hand_shape);

  /** Below for debug **/
  //cout << my_pos << endl;
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

  output(mc_play(last_hand));
}
