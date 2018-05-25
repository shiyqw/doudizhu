#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <functional>
#include <ctime>
#include <cstdlib>
#include <list>
#include <fstream>

#include "jsoncpp/json.h"

using namespace std;

/*****************************************************************************/
// Global Data
/*****************************************************************************/
int MC_GAME_NUMBERS = 5000;
double ELAPSED_SECS = -1.;
set<int> public_cards;
int mc_rem[3];
int mc_shape[3][15];
int score[3];
int shape[3][15];
int origin[3][15];
int rem[3];
vector< vector<int> > history;
vector<double> real_time_win_rate;
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

  void show() {
      stringstream buffer;
      buffer << point << ":" << width << "x" << length;
      for (auto card : carry) {
          buffer << " " << card;
      }
      cout << setw(20) << left << buffer.str();
  }

};


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

vector<int> mc_shuffle(int pos) {
  vector<int> cards;
  for (int i = 0; i < 15; ++i) {
    for (int k = 1; k < 3; ++k) {
      for (int j = 0; j < shape[(pos+k)%3][i]; ++j) {
        cards.push_back(i);
      }
    }
  }
  random_shuffle(cards.begin(), cards.end());
  return cards;
}


void mc_init(int pos, vector<int> cards) {
  for (int i = 0; i < 3; ++i) {
    mc_rem[i] = rem[i];
    score[i] = 0;
  }
  // Shuffle remaining cards;
  for (int i = 0; i < 15; ++i) {
    mc_shape[pos][i] = shape[pos][i];
  }
  for (int i = 1; i < 3; ++i) {
    for (int j = 0; j < 15; ++j) {
      mc_shape[(pos+i)%3][j] = 0;
    }
  }
  for (int i = 0; i < rem[(pos+1)%3]; ++i) {
    mc_shape[(pos+1)%3][cards[i]]++;
  }
  for (int i = 0; i < rem[(pos+2)%3]; ++i) {
    mc_shape[(pos+2)%3][cards[i+rem[(pos+1)%3]]]++;
  }
}

int mc_get_length_with_width(int pos, int left, int width) {
  int right_limit = width >= 3 ? 13 : 12;
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
int get_length_with_width(int pos, int left, int width) {
  int right_limit = width >= 3 ? 13 : 12;
  for (int i = left; i <= right_limit; ++i) {
    if (shape[pos][i] < width || i == right_limit) {
      return (i-left);
    }
  }
  return 0;
}


void redo(int pos, Hand hand) {
  if (hand.is_pass()) {
    return;
  }
  for (int i = hand.point; i < hand.point + hand.length; ++i) {
    shape[pos][i] -= hand.width;
    rem[pos] -= hand.width;
  }
  for (auto card : hand.carry) {
    shape[pos][card]--;
    rem[pos]--;
  }
}

void undo(int pos, Hand hand) {
  if (hand.is_pass()) {
    return;
  }
  for (int i = hand.point; i < hand.point + hand.length; ++i) {
    shape[pos][i] += hand.width;
    rem[pos] += hand.width;
  }
  for (auto card : hand.carry) {
    shape[pos][card]++;
    rem[pos]++;
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
                if (i == 12 && width == 4) {
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
int mc_run(int pos, Hand prev_hand, Hand prev_prev_hand, int pass_num) {

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

  int position = (pos + 1) % 3;
  int pass = 0;
  if (prev_hand.is_pass()) {
    prev_hand = prev_prev_hand;
    pass += 1;
    if (pass_num == 1) {
      prev_hand = Hand();
    }
  }
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
Hand one_hand(int pos, Hand prev, int pass) {
  clock_t start_time = clock();
  auto max_hand = Hand();
  list<pair<Hand, int>> hands;
  double max_win_rate = -10000.;

  /** Search Strings, 3+2 and 3+1 **/
  for (auto left = 0; left < 13; ++left) {
    for (auto width = 1; width <= 4; ++width) {
      auto length = get_length_with_width(pos, left, width);
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
              if (shape[pos][i] < carry_width) {
                continue;
              }
              if (i == 12 && width == 4) {
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
    for (int width = 1; width <= shape[pos][i] && width <= 2; ++width) {
      Hand pure_hand = Hand(i, width, 1);
      //pure_hand.show();
      if (check_valid(pure_hand, prev)) {
        hands.push_back(make_pair(pure_hand, 0));
      }
    }
  }

  /** Search Bomb **/
  if ((rem[0] <= 6 || rem[1] <= 6 || rem[2] <= 6)
        && prev.length != 0) {
    for (int i = 0; i < 13; ++i) {
      if (shape[pos][i] == 4) {
        Hand bomb_hand = Hand(i, 4, 1);
        //bomb_hand.show();
        if (check_valid(bomb_hand, prev)) {
          hands.push_back(make_pair(bomb_hand, 0));
        }
      }
    }
    if (shape[pos][13] == 1 && shape[pos][14] == 1) {
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

  if (hands.size() == 1) {
    // Play itself
    int game_number = MC_GAME_NUMBERS;
    for (int j = 0; j < game_number; ++j) {
      auto cards = mc_shuffle(pos);
      for (auto it = hands.begin(); it != hands.end(); ++it) {
        auto hand = it->first;
        redo(pos, hand);
        mc_init(pos, cards);
        mc_calc_detail(pos, hand);
        int win_pos = mc_run(pos, hand, prev, pass);
        if (pos == 0) {
          int delta = 2*score[0] - (score[1]+score[2]);
          it->second += delta;
          if (win_pos == 0) {
            it->second += 400;
          } else {
            it->second -= 400;
          }
        } else if (pos != 0) {
          int delta = (score[1]+score[2]) - 2*score[0];
          it->second += delta;
          if (win_pos == 0) {
            it->second -= 400;
          } else {
            it->second += 400;
          }
        }
        undo(pos, hand);
      }
    }
    max_hand = hands.front().first;
    double win_rate = (double) hands.front().second / 200. / (double) MC_GAME_NUMBERS;
    real_time_win_rate.push_back(win_rate);
  } else {
    for (int i = 1; i <= k-1; ++i) {
      int game_number = n[i]-n[i-1];
      for (int j = 0; j < game_number; ++j) {
        auto cards = mc_shuffle(pos);
        for (auto it = hands.begin(); it != hands.end(); ++it) {
          auto hand = it->first;
          redo(pos, hand);
          mc_init(pos, cards);
          mc_calc_detail(pos, hand);
          int win_pos = mc_run(pos, hand, prev, pass);
          if (pos == 0) {
            int delta = 2*score[0] - (score[1]+score[2]);
            it->second += delta;
            if (win_pos == 0) {
              it->second += 400;
            } else {
              it->second -= 400;
            }
          } else if (pos != 0) {
            int delta = (score[1]+score[2]) - 2*score[0];
            it->second += delta;
            if (win_pos == 0) {
              it->second -= 400;
            } else {
              it->second += 400;
            }
          }
          undo(pos, hand);
        }
        clock_t current_time = clock();
        ELAPSED_SECS = double (current_time - start_time) / CLOCKS_PER_SEC;
        //if (ELAPSED_SECS > 0.9) {
        //  int max_win = -0x7FFFFFFF;
        //  list<pair<Hand, int>>::iterator best_hand;
        //  for (auto it = hands.begin(); it != hands.end(); ++it) {
        //    if (it->second > max_win) {
        //      best_hand = it;
        //      max_win = it->second;
        //    }
        //  }

        //  max_hand = best_hand->first;
        //  return max_hand;
        //}
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

      //cout << "used seconds:" << ELAPSED_SECS << endl;

      if (hands.size() > 1) {
        hands.erase(worst_hand);
      }
    }

    max_hand = hands.front().first;
    double win_rate = (double) hands.front().second / 200. / (double) n[k-1];
    real_time_win_rate.push_back(win_rate);
  }
  return max_hand;
}

/*****************************************************************************/
// Main Program: Get Input, Get Recent Game Status, Start Search and Output
/*****************************************************************************/
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
    int prev_pos = (my_pos + 2) % 3;
    int next_pos = (my_pos + 1) % 3;
    for (int j = 0; j < 15; ++j) {
      output.push_back(origin[prev_pos][j] - history[i][prev_pos*15+j]);
    }
    for (int j = 0; j < 15; ++j) {
      output.push_back(origin[next_pos][j] - history[i][next_pos*15+j]);
    }
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

    int j = 14;
    int control1 = 0;
    int control2 = 0;
    int control3 = 0;
    for (; j >= 0; --j) {
      if (history[i][prev_pos*15+j] > 0) {
        break;
      }
      if (history[i][next_pos*15+j] > 0) {
        break;
      }
      control1 += output[j];
    }

    for (j = 14; j >= 0; --j) {
      if (history[i][prev_pos*15+j] + history[i][next_pos*15+j] >= 2) {
        break;
      }
      control2 += output[j] >= 2 ? 2 : 0;
    }

    for (j = 14; j >= 0; --j) {
      if (history[i][prev_pos*15+j] + history[i][next_pos*15+j] >= 3) {
        break;
      }
      control3 += output[j] >= 3 ? 3 : 0;
    }

    output.push_back(control1);
    output.push_back(control2);
    output.push_back(control3);

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
      dizhu_file << real_time_win_rate[i] << endl;
    } else if (my_pos == 1) {
      xiajia_file << real_time_win_rate[i] << endl;
    } else {
      menban_file << real_time_win_rate[i] << endl;
    }
  }
  dizhu_file.close();
  menban_file.close();
  xiajia_file.close();
}

void output_shape() {
  vector<int> status;
  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 15; ++i) {
      status.push_back(shape[j][i]);
    }
  }
  history.push_back(status);
}

void random_start() {
  history.clear();
  real_time_win_rate.clear();
  public_cards.clear();
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
  for (int i = 0; i < 54; ++i) {
    cards.push_back(i);
  }
  random_shuffle(cards.begin(), cards.end());
  for (int i = 0; i < 20; ++i) {
    shape[0][card_to_point(cards[i])]++;
    origin[0][card_to_point(cards[i])]++;
  }
  for (int i = 20; i < 37; ++i) {
    shape[1][card_to_point(cards[i])]++;
    origin[1][card_to_point(cards[i])]++;
  }
  for (int i = 37; i < 54; ++i) {
    shape[2][card_to_point(cards[i])]++;
    origin[2][card_to_point(cards[i])]++;
  }
}

int main() {
  std::srand(unsigned(std::time(0)));
  for (int k = 0; k < 1000; ++k) {
    cout << k << endl;
    random_start();
    Hand prev_hand = Hand();
    int position = 0;
    int pass = 0;
    while(rem[0] != 0 && rem[1] != 0 && rem[2] != 0) {
      Hand hand = one_hand(position, prev_hand, pass);
      redo(position, hand);
      if (hand.length == 0) {
        pass += 1;
      } else {
        prev_hand = hand;
        pass = 0;
      }
      if (pass == 2) {
        prev_hand = Hand();
      }
      //cout << position << " Go ";
      //hand.show();
      //cout << endl;
      //cout << rem[0] << ' ' << rem[1] << ' ' << rem[2] << endl;
      position = (position + 1) % 3;
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

