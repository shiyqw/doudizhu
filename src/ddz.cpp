#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <functional>
#include <ctime>
#include <cstdlib>
#include <list>
#include <string>

#include "jsoncpp/json.h"

using namespace std;
bool local_debug = false;

/*****************************************************************************/
// Global Data
/*****************************************************************************/
int MC_GAME_NUMBERS = 4000;
double ELAPSED_SECS = -1.;
set<int> my_cards;
set<int> last_hand_cards;
bool prev_pass;
set<int> public_cards;
set<int> remain_cards;
vector<int> last_hand_shape;
vector<int> my_shape = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int remain_shape[15] = {4,4,4,4,4,4,4,4,4,4,4,4,4,1,1};
int rem[3] = {20,17,17};
int mc_rem[3];
int mc_shape[3][15];
int my_pos;
int last_hand_pos = -1;
int score[3];
vector<int> prev_predict, next_predict;
vector<vector<int> > prev_history, next_history;
clock_t start_time;
bool enemy_is_ordered = true;
stringstream debug_buffer;
int encode_mapping[4] = {3,1,2,0};

//const int shuffle_times = 1;

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

  int card_num() {
    return width*length+carry.size();
  }

  void show() {
      stringstream buffer;
      buffer<<point << ":" << width << "x" << length;
      for (auto card : carry) {
          buffer << " " << card;
      }
      debug_buffer << buffer.str();
  }

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
  if (hand.is_pass()) {
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

/*****************************************************************************/
// Weights and Calculations for Game Evaluation
/*****************************************************************************/
double weight[3][23][19] = { {
  {2.023,3.5706,2.2008,2.3242,2.2191,2.1362,2.7244,1.8046,1.7442,1.4918,1.3216,0.25995,0.33808,-0.71847,-0.31956,-0.22541,-1.3252,-2.2842,-0.9693},
  {-0.85705,-0.24624,-0.29491,-0.21583,-0.12549,-0.048176,-0.079576,-0.029953,-0.056868,0.010422,-0.034699,0.048968,0.034655,0.10584,0.026144,0.0081571,0.0043943,0.068843,-0.038258},
  {-1.8109,-0.50161,-0.42605,-0.2885,-0.10373,-0.0048157,-0.054343,0.037318,0.13611,0.095014,-0.0032154,0.1221,0.032492,0.1494,0.094838,0.11216,0.12667,0.13888,0.0065012},
  {-1.7207,-0.50583,-0.26912,-0.23117,-0.12181,-0.056662,0.0072395,0.046043,0.085728,0.024582,0.023079,0.030104,0.075752,0.11445,0.0082531,0.00033101,0.058781,0.089242,0.0089628},
  {-1.9925,-0.54407,-0.38514,-0.17414,-0.11737,0.001235,0.027769,0.056375,0.0075807,0.058028,-0.010448,0.036488,0.04853,0.13942,0.038679,0.027543,0.11752,0.093566,0.031146},
  {-2.2806,-0.59027,-0.39133,-0.28861,-0.18513,-0.017249,-0.00555,0.094352,0.079852,0.073573,0.077052,0.09076,0.027251,0.13292,0.07751,0.015135,0.087305,0.016993,-0.045782},
  {-2.273,-0.63202,-0.477,-0.29017,-0.19811,-0.061124,-0.12343,0.062188,0.089064,0.032156,0.0031984,0.11269,0.11695,0.12706,0.030546,0.066201,0.13922,0.071286,-0.0077139},
  {-2.2324,-0.51289,-0.41003,-0.257,-0.16615,-0.097657,-0.099134,0.0069653,-0.028666,0.0081443,-0.057444,0.023774,-0.015807,0.064004,0.019378,0.039325,0.090803,0.048993,-0.017668},
  {-1.9083,-0.46198,-0.43964,-0.27373,-0.093769,-0.0040398,-0.027129,0.00023766,0.0073612,-0.010137,-0.010528,0.059325,0.034227,0.099203,0.03189,0.045383,0.11074,0.093919,0.033546},
  {-1.6528,-0.40314,-0.13815,-0.16632,-0.032054,0.013968,0.015671,0.035558,0.036542,0.040782,0.058561,0.045886,0.096708,0.16416,0.060193,0.14411,0.20583,0.11501,0.023031},
  {-1.4608,-0.29012,-0.2581,-0.1479,-0.087108,-0.0022858,0.020569,0.13379,0.090249,0.084228,0.052193,0.10218,0.075057,0.17132,0.04362,0.076089,0.1117,0.15559,0.056333},
  {-1.0799,-0.23354,-0.055163,0.021763,0.062901,0.12273,0.088445,0.11951,0.18877,0.13211,0.1245,0.17053,0.16686,0.27477,0.15752,0.21994,0.20422,0.25555,0.15315},
  {-0.95275,0.2534,0.38022,0.31464,0.26436,0.34468,0.30378,0.39085,0.3834,0.38079,0.31485,0.4092,0.40204,0.4906,0.36552,0.49308,0.5919,0.63851,0.3527},
  {-0.27441,0.91009,1.0172,1.0398,1.052,1.1863,1.1258,1.1707,1.0873,1.0046,0.94941,0.91887,0.88059,0.79609,0.84387,0.85107,1.267,0.87068,0.93107},
  {0.92671,1.6275,1.4353,1.1559,0.98737,1.1947,0.99276,1.2716,1.1726,0.87493,0.91291,1.0174,0.75997,0.80978,0.75537,0.76018,0.59021,0.51443,0.89847},
  {0,0,0,0,0.57324,0.64268,0.59843,0.49605,0.43842,0.35898,0.32812,0.24373,0.18793,0.16455,0.11825,0.14049,0.14231,0.064472,0.056576},
  {0,0,0,0,0,0.40361,0.48213,0.34991,0.28506,0.26017,0.19023,0.15586,0.068805,0.05044,0.06863,0.041375,0.057704,0.029596,0.07046},
  {0,0,0,0,0,1.0359,-0.052706,1.5046,0.47356,0.48684,0.36865,0.51893,0.46022,0.32558,0.35993,0.32091,0.28708,0.36066,0.34778},
  {0,-1.6998,-0.85353,-0.82586,-0.75731,-0.79784,-0.83573,-0.70502,-0.66571,-0.57942,-0.49881,-0.41887,-0.38454,-0.36826,-0.28183,-0.31847,-0.29773,-0.18248,-0.16829},
  {0,-0.2247,1.0269,1.1874,1.3938,0.8279,0.64015,0.6498,0.41843,0.37628,0.5792,0.42216,0.21311,0.34758,0.49422,0.44752,0.42702,0.35095,0.52011},
  {-1.014,0.2216,0.33481,0.33829,0.35664,0.43952,0.49212,0.36798,0.4187,0.45499,0.35735,0.35172,0.45858,0.41787,0.23737,0.19845,0.31399,0.34904,0.035285},
  {0,-0.64053,-0.099872,-0.17359,-0.092658,-0.02141,0.058136,-0.016789,-0.010577,-0.039822,0.0035939,0.019836,0.0053658,-0.021116,0.06056,-0.0047366,0.036479,0.0053862,-0.0069181},
  {0,0,-0.034921,0.06141,0.14912,0.13325,0.053396,0.079082,0.036522,0.077371,0.064693,0.05582,0.0046805,0.038845,0.0085624,0.021941,-0.032031,-0.0073683,0.05095}
},{
  {1.7337,1.9464,1.6327,1.4578,1.5795,1.0793,0.81479,0.62642,0.5442,0.027298,0.024115,-0.34625,-0.46963,-0.46429,-1.0721,-0.64682,-0.72181,0,0},
  {-0.12873,0.42071,-0.041905,0.029138,-0.13523,0.010188,0.022741,-0.01398,-0.01744,0.020537,0.0068734,0.022481,0.032773,0.055091,0.052726,0.017383,0.01769,0,0},
  {-0.22628,0.13472,0.00092433,0.064329,-0.040142,0.02416,0.031693,0.010519,0.0020086,0.014393,0.023693,0.054684,0.026128,0.054427,0.047682,0.021194,0.027301,0,0},
  {-0.25178,0.041658,-0.05971,0.062678,-0.15541,-0.032546,0.089842,-0.015827,0.017905,-0.00085868,0.0060146,0.017652,0.018487,0.020971,0.047173,0.016499,0.027258,0,0},
  {-0.63548,0.11147,-0.10685,-0.04462,-0.11465,-0.030108,0.064011,0.0086654,0.021824,0.041234,0.012856,0.03118,0.027724,0.012348,0.038259,0.022199,0.014412,0,0},
  {-0.86177,0.07571,-0.11556,-0.029882,-0.12473,-0.070662,0.089215,0.022668,0.056309,0.056726,0.016928,0.023731,0.031097,0.056538,0.05663,0.016451,0.028693,0,0},
  {-0.85581,-0.018213,-0.029114,-0.068109,-0.12068,-0.04162,0.0024638,-0.039533,0.0019711,0.012159,-0.0023824,0.029494,0.0087589,-0.0014686,0.031315,-0.010843,0.0092857,0,0},
  {-0.78395,-0.013184,-0.16112,-0.0981,-0.095985,-0.0503,0.0094287,-0.076701,-0.011154,0.010451,-0.01266,0.0005273,0.015238,-0.01695,0.026887,-0.002188,0.0071472,0,0},
  {-0.8133,0.047639,-0.20639,-0.069472,-0.15607,-0.053987,-0.02438,-0.053537,-0.027378,-0.005164,0.035042,0.017636,0.036109,0.062293,0.044251,0.017707,0.040103,0,0},
  {-0.83166,0.086353,-0.13294,-0.090448,-0.098314,-0.042007,0.05474,0.019481,-0.0039646,-0.0069274,0.014225,0.011908,0.048866,0.020529,0.058945,0.028778,0.03584,0,0},
  {-0.7449,-0.013225,-0.22053,-0.066275,-0.13639,-0.071661,0.014829,-0.04718,-0.031526,0.036436,0.0045177,0.024732,0.061367,0.079392,0.075378,0.050005,0.054718,0,0},
  {-0.61835,0.029761,-0.097856,-0.054476,-0.14803,-0.0080885,0.016527,0.014465,0.03382,0.050242,0.059628,0.050527,0.12329,0.10433,0.11794,0.07699,0.091284,0,0},
  {-0.27181,0.34117,0.096245,0.18493,-0.059372,0.033676,0.10833,0.077647,0.10812,0.15288,0.15031,0.13762,0.16042,0.15581,0.18168,0.15031,0.13284,0,0},
  {0.18127,0.7214,0.34236,0.31796,0.23418,0.16817,0.32655,0.22869,0.22042,0.26229,0.208,0.19178,0.1969,0.26024,0.22444,0.22217,0.17669,0,0},
  {1.014,1.4761,0.93232,0.575,0.47865,0.46603,0.53164,0.35345,0.30733,0.37738,0.32235,0.31849,0.29338,0.34806,0.30122,0.2965,0.23935,0,0},
  {0,0,0,0,0.44736,0.37428,0.22267,0.17956,0.14112,0.097673,0.065058,0.039577,0.040081,0.031329,0.015264,0.010262,0.0077629,0,0},
  {0,0,0,0,0,0.59273,0.43548,0.17796,0.1601,0.073054,0.058211,0.0047821,0.033404,-0.018505,0.0010858,-0.003048,-0.0095934,0,0},
  {0,0,0,0,0,0.72757,0,1.1949,0.8445,0.08049,0.058393,0.0028709,0.014969,0.015277,0.063525,0.018047,0.027074,0,0},
  {0,-0.8135,-0.36247,-0.33162,-0.20422,-0.17412,-0.18372,-0.094234,-0.10339,-0.05706,-0.057318,-0.024731,-0.031656,-0.030891,-0.0034093,-0.0065297,-0.020954,0,0},
  {0,0.41892,1.2374,1.5999,1.0902,0.91096,0.27569,0.34734,0.51061,0.33377,0.42164,0.26113,0.31293,0.33383,0.18548,0.10912,0.16853,0,0},
  {-0.44042,0.079151,0.076954,0.069785,0.02453,0.12428,0.097295,0.099436,0.077622,0.025246,0.047596,0.039773,0.021404,-0.032622,0.022259,-0.022811,0.017081,0,0},
  {0,-0.047516,0.10623,0.0052489,0.084274,0.034708,0.040524,0.042471,0.024292,0.023384,0.024802,0.022532,0.022832,0.0057464,0.017022,0.011635,0.013094,0,0},
  {0,0,0.11001,0.2728,0.19335,0.11629,0.019491,0.05152,0.062449,0.055255,0.046564,0.059002,0.034177,0.028056,0.031181,0.031872,0.014322,0,0}
},{
  {1.0469,2.0486,1.2504,1.252,1.1967,0.9086,1.0418,0.558,0.37843,0.21969,0.072432,0.20638,0.098971,0.35061,0.11828,0.052704,0.031354,0,0},
  {-0.10706,-0.30501,-0.14092,0.043861,-0.082465,0.039453,-0.027007,0.00027888,0.023797,-0.0025932,0.030344,-0.019474,-0.015278,0.016877,-0.016062,-0.0087557,-0.017828,0,0},
  {0.068954,-0.19887,-0.042442,-0.023612,-0.077112,0.0017778,-0.033979,-0.00070924,0.021114,-0.0021979,0.0081635,-0.019619,-0.0029044,0.011554,-0.023008,-0.012332,-0.018573,0,0},
  {-0.023991,-0.18501,-0.067912,-0.075292,-0.091121,-0.020157,-0.044041,-0.0063557,0.01393,-0.026417,-0.019461,-0.034787,-0.031825,-0.031064,-0.017584,-0.0055838,-0.014287,0,0},
  {-0.1449,-0.34107,-0.072567,-0.063702,-0.03874,-0.042937,-0.02745,0.010445,0.03712,0.024207,-0.0076136,-0.040928,-0.019792,-0.02698,-0.030522,-0.015171,-0.0016562,0,0},
  {-0.11198,-0.32662,-0.073948,-0.059349,-0.044767,-0.041492,-0.014827,-0.019102,0.00026027,-0.02238,-0.016206,-0.055918,-0.049952,-0.011999,-0.026488,-0.035772,-0.023615,0,0},
  {0.12589,-0.15921,-0.040733,-0.034349,-0.061769,0.010904,-0.044505,-0.0058656,0.018522,-0.0021695,-0.016877,-0.033649,-0.010812,-0.015808,-0.0076993,-0.0064492,0.0023186,0,0},
  {0.37434,-0.14335,-0.027009,-0.01593,-0.069968,0.016977,-0.052173,-0.0056418,-0.029848,-0.023041,-0.0086329,-0.046679,-0.018799,0.0055746,-0.0016254,-0.0032444,-0.0019702,0,0},
  {0.24047,-0.11002,0.063001,0.064455,-0.044365,0.026294,-0.01178,0.013958,0.022496,0.0090433,0.0093484,-0.01597,0.012628,-0.035115,0.0013413,-0.0090552,-0.00034819,0,0},
  {0.27688,-0.083036,0.028139,0.032121,-0.022481,0.031857,-0.020838,0.015278,0.011624,0.0095064,0.031649,-0.021941,0.033997,0.010579,0.00074505,0.011428,2.2583e-05,0,0},
  {0.66655,-0.0048812,0.13491,0.05468,-0.00192,0.0011922,0.0078403,-0.0068603,0.036286,0.023949,0.031916,0.0062843,0.026517,0.0061305,0.021091,0.0099183,0.015058,0,0},
  {0.65078,0.13494,0.23277,0.12993,0.093964,0.11534,0.096646,0.11455,0.13899,0.10938,0.10655,0.069353,0.10015,0.090801,0.036693,0.029246,0.02672,0,0},
  {0.79882,0.54753,0.54689,0.38107,0.23774,0.23059,0.17554,0.17116,0.17502,0.12743,0.16027,0.088217,0.13293,0.10587,0.069619,0.068379,0.030226,0,0},
  {1.2076,1.0361,0.82672,0.64912,0.42401,0.41539,0.33523,0.40097,0.23949,0.21335,0.21398,0.16908,0.154,0.19517,0.10789,0.14197,0.025422,0,0},
  {1.6549,1.6486,1.0348,0.7504,0.47607,0.45842,0.45445,0.372,0.19297,0.13426,0.16269,0.085571,0.14716,0.30266,0.12655,0.15301,-0.11068,0,0},
  {0,0,0,0,0.50756,0.38063,0.24614,0.13384,0.092216,0.070502,0.062374,0.045768,0.041271,0.031719,0.016024,0.0067847,0.0074889,0,0},
  {0,0,0,0,0,0.39416,0.3463,0.22881,0.1538,0.04567,0.024095,0.026037,0.027936,-0.0021047,0.016823,0.0047889,0.0073067,0,0},
  {0,0,0,0,0,0,0,0,-0.090314,-0.08646,0.056887,0.0013857,-0.069392,-0.099381,-0.020538,-0.010698,0.0221,0,0},
  {0,-0.77569,-0.45372,-0.38397,-0.25456,-0.23821,-0.20764,-0.14931,-0.12722,-0.078297,-0.064448,-0.036614,-0.058253,-0.076684,-0.041309,-0.030854,-0.032881,0,0},
  {0,-0.70279,0.97647,0.99851,0.96138,0.51677,0.18803,0.19971,0.26418,0.34076,0.42742,0.32325,0.31915,0.39679,0.17827,0.02309,0.11297,0,0},
  {-0.31949,0.15022,0.20579,0.19405,0.080055,0.10579,0.10797,0.092798,0.16745,0.061599,0.014282,0.017097,0.01996,-0.1599,-0.0098731,-0.0149,0.11274,0,0},
  {0,-0.093476,-0.067667,-0.029039,-0.005321,0.017855,0.0033731,0.012339,0.016037,0.01079,0.014092,0.014335,0.013307,-0.0045436,0.011507,-0.0045103,0.022033,0,0},
  {0,0,0.066841,0.1032,0.15241,0.0063559,-0.0014265,-0.0079087,-0.0025887,0.0030005,-0.0027998,0.0097001,0.0075327,-0.0012424,0.0023575,0.0014341,0.0056775,0,0}
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
  int remain_number = mc_rem[pos];
  /** Cannot Happen **/
  if (remain_number == 20) {
    return -10000.;
  }
  for (int i = 0; i < features.size(); ++i) {
    value += features[i] * weight[pos][i][remain_number];
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
  if (my_pos == 0) {
    vector<int> shuffled_cards;
    for (auto card : my_cards) {
      shuffled_cards.push_back(card);
    }
    random_shuffle(shuffled_cards.begin(), shuffled_cards.end());
    for (auto card : shuffled_cards) {
      auto point = card_to_point(card);
      if (shape[point] > 0) {
        cards.push_back(card);
        shape[point]--;
      }
    }
    return cards;
  }
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

/**
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
**/

void mc_shuffle(vector<int> & cards) {
  int prev_fixed_card_num = prev_predict.size();
  int next_fixed_card_num = next_predict.size();
  if (my_pos == 1) {
    prev_fixed_card_num += public_cards.size();
  } else if (my_pos == 2) {
    next_fixed_card_num += public_cards.size();
  }
  auto start_pos = cards.begin() + next_fixed_card_num;
  auto end_pos = cards.end() - prev_fixed_card_num;
  random_shuffle(start_pos, end_pos);
}

vector<int> mc_distribute() {
  vector<int> cards;
  for (auto card : next_predict) {
    cards.push_back(card_to_point(card));
  }
  if (my_pos == 2) {
    for (auto card : public_cards) {
      cards.push_back(card_to_point(card));
    }
  }
  for (auto card : remain_cards) {
    cards.push_back(card_to_point(card));
  }
  for (auto card : prev_predict) {
    cards.push_back(card_to_point(card));
  }
  if (my_pos == 1) {
    for (auto card : public_cards) {
      cards.push_back(card_to_point(card));
    }
  }
  return cards;
}

void mc_init(const vector<int> & cards) {
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
  if (left == 12) {
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
  if (left == 12) {
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

          if (!check_valid(string_hand, prev) || string_hand.width == 4) {
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
if (true) {
  //if (mc_rem[0] <= 6 || mc_rem[1] <= 6 || mc_rem[2] <= 6) {
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

  /** Skim **/
  for (auto p : hands) {
    if (p.first.card_num() == mc_rem[pos]) {
      mc_redo(pos, p.first);
      return p.first;
    }
  }
  /** Randomized **/
  random_number = min(random_number, (int) hands.size());
  if (!hands.empty()) {
    auto rand_result = rand() % random_number;
    for (int i = 0; i < rand_result; ++i) {
      hands.erase(max_element(hands.begin(), hands.end(), my_comp));
    }
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
int mc_run(Hand prev_hand, Hand prev_prev_hand, bool prev_pass) {

  if (local_debug) {
    cout << endl;
    cout << "start run" << endl;
    cout << prev_hand.point << endl;
    //prev_hand.show();
    cout << endl;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 15; ++j) {
            cout << mc_shape[i][j] << " ";
        }
        cout << endl;
    }
    cout << "********************" << endl;
  }

  int position = (my_pos + 1) % 3;
  int pass = 0;
  if (prev_hand.is_pass()) {
    prev_hand = prev_prev_hand;
    pass += 1;
    if (prev_pass) {
      prev_hand = Hand();
    }
  }

  while(mc_rem[0] != 0 && mc_rem[1] != 0 && mc_rem[2] != 0) {
    int random_number = 1;

    /** Below should be toggled if we want add random factors **/
    //if (rem[0] + rem[1] + rem[2] <= 25) {
    //    random_number = 3;
    //}
    if (rem[0] <= 5 || rem[1] <= 5 || rem[2] <= 5) {
        random_number = 3;
    }

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

    if (local_debug) {
      for (int i = 0; i < 3; ++i) {
          for (int j = 0; j < 15; ++j) {
            cout << mc_shape[i][j] << " ";
          }
          cout << endl;
      }
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }

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
vector<int> mc_play(Hand prev, bool prev_pass) {
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

        if (!check_valid(string_hand, prev) || string_hand.width == 4) {
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
    int final_candidates = 5;
    for(int i = max(k-final_candidates+1,1); i<k; ++i){
        n[i] = 1e8 * max(i-k+final_candidates +1,1);
    }


    auto cards = mc_distribute();
    double hands_searched = 0.0;
    /*clock_t current_time = clock();
    double BEGIN_SECS = double (current_time - start_time) / CLOCKS_PER_SEC;
    double SHUFFLE_SEC ;*/
    for (int i = 1; i <= k-1; ++i) {
      int game_number = n[i]-n[i-1];
      for (int j = 0; j < game_number; ++j) {
        /** Init card distribution with predict **/
        vector<int> worst_shuffle;
        double worst_score = -10000.;
        mc_shuffle(cards);
        //for(int shuffle = 0; shuffle < shuffle_times; ++shuffle) {
        //  mc_shuffle(cards);
        //  auto one_shuffle = vector<int>(cards);
        //  mc_init(one_shuffle);
        //  vector<double> shuffle_score(3, 0.);
        //  for (int pos = 0; pos < 3; ++pos) {
        //    shuffle_score[pos] = mc_evaluate(pos);
        //  }
        //  double temp_score;
        //  if (my_pos == 0) {
        //    temp_score = max(shuffle_score[1], shuffle_score[2]);
        //  } else {
        //    temp_score = shuffle_score[0];
        //  }
        //  if (temp_score > worst_score) {
        //    worst_score = temp_score;
        //    worst_shuffle = one_shuffle;
        //  }
        //}

        /*if(i==1 && j ==0){
            clock_t current_time = clock();
            SHUFFLE_SEC = double (current_time - start_time) / CLOCKS_PER_SEC -BEGIN_SECS;
        }*/
        //cout << "shuffle result:" << endl;
        //for (auto card : cards) {
        //  cout << card << ',';
        //}
        //cout << endl;

        for (auto it = hands.begin(); it != hands.end(); ++it) {
          //++hands_searched;
          auto hand = it->first;
          redo(hand);
          mc_init(cards);
          mc_calc_detail(my_pos, hand);
          int win_pos = mc_run(hand, prev, prev_pass);
          //cout << hand.width << endl;
          //cout << "score : " << score[0] << ',' << score[1] << ',' << score[2] << endl;
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
        /*
        ///estimate rounds begin///
        if (i==1 && j == 0) {
            int rounds = ceil((0.9-BEGIN_SECS) / ((ELAPSED_SECS-BEGIN_SECS - SHUFFLE_SEC) /hands_searched + SHUFFLE_SEC/logk));
            if(rounds < 5000)
                rounds = 5000;
            for (int r = 1; r < k; ++r) {
            n[r] = (int) ceil(1.0 / logk * ((double) (rounds-k)) / ((double) (k+1-r)));
            }
            game_number = n[1]-n[0];
        }
        ///estimate rounds end///
        */
        if (ELAPSED_SECS > 0.95) {
          int max_win = -0x7FFFFFFF;
          list<pair<Hand, int>>::iterator best_hand;
          for (auto it = hands.begin(); it != hands.end(); ++it) {
            if (it->second > max_win) {
              best_hand = it;
              max_win = it->second;
            }
          }
          /// debug begin here
          debug_buffer << "| TOP k: ";
          for (auto hand_with_win : hands) {
            hand_with_win.first.show();
            debug_buffer << " : " << hand_with_win.second / 200. / (double) (n[i-1]+j) << " , " ;
          }
          debug_buffer << " | ";
          /// debug end here
          max_hand = best_hand->first;
          return hand_to_card(max_hand);
        }
      }
      int min_win = 0x7FFFFFFF;
      list<pair<Hand, int>>::iterator worst_hand, it;

      /*
      for (auto hand_with_win : hands) {
          hand_with_win.first.show();
          debug_buffer << " : " << hand_with_win.second / 200. / (double) n[i] << " , " ;
      }
      debug_buffer << " | ";
      */

      for (auto it = hands.begin(); it != hands.end(); ++it) {
        if (it->second < min_win) {
          worst_hand = it;
          min_win = it->second;
        }
      }
      if(i >= k - 30){
        worst_hand->first.show();
      debug_buffer <<setprecision(3)<<fixed<< " : " << min_win / 200. / (double) n[i] << " "<<ELAPSED_SECS<<"s,";
      }

      hands.erase(worst_hand);
    if(i==k-1){
        worst_hand = hands.begin();
        debug_buffer <<"RES:";
        min_win = worst_hand->second;
        worst_hand->first.show();
        debug_buffer <<setprecision(3)<<fixed<< " : " << min_win / 200. / (double) n[i] << " , " ;
    }
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

  // Init for predict
  prev_history = vector<vector<int>>(15, vector<int>());
  next_history = vector<vector<int>>(15, vector<int>());
  prev_predict = vector<int>();
  next_predict = vector<int>();
  for (int i = 0; i < 54; ++i) {
    remain_cards.insert(i);
  }

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
      remain_cards.erase(card);
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
  prev_pass = false;

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
        if (pos == 1) {
          // previous player
          prev_history[card_to_point(card)].push_back(card%4);
        } else {
          next_history[card_to_point(card)].push_back(card%4);
        }
        remain_cards.erase(card);
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
        } else {
          prev_pass = true;
        }
      }
    }
  }

  my_shape = card_to_shape(my_cards);
  if (my_pos != 0) {
    for (auto card : public_cards) {
      remain_cards.erase(card);
    }
  }
}

void output(vector<int> cards) {
  Json::Value result, response(Json::arrayValue);
  for (auto card : cards) {
    response.append(card);
  }
  result["response"] = response;
  debug_buffer << "Elapsed Time: " << ELAPSED_SECS << " | ";
  result["debug"] = debug_buffer.str();
  Json::FastWriter writer;
  cout << writer.write(result) << endl;
}

/*****************************************************************************/
// Predict distribution
/*****************************************************************************/
void get_predict() {
  if (my_pos == 0) {
    for (auto cards : prev_history) {
      int last_card = -1;
      for (auto card : cards) {
        if (card > last_card) {
          last_card = card;
        } else {
          enemy_is_ordered = false;
          break;
        }
      }
    }
    if (enemy_is_ordered) {
      for (auto cards : next_history) {
        int last_card = -1;
        for (auto card : cards) {
          if (card > last_card) {
            last_card = card;
          } else {
            enemy_is_ordered = false;
            break;
          }
        }
      }
    }
    if (!enemy_is_ordered) {
      return;
    }
    for (int i = 0; i < 13; ++i) {
      auto cards = prev_history[i];
      int last_card = -1;
      for (auto card : cards) {
        if (card - last_card > 1) {
          for (int j = 1; j < card - last_card; ++j) {
            int skipped_card = 4*i + last_card + j;
            if (remain_cards.count(skipped_card) == 1) {
              next_predict.push_back(skipped_card);
              remain_cards.erase(skipped_card);
            }
          }
          last_card = card;
        }
      }
    }
    for (int i = 0; i < 13; ++i) {
      auto cards = next_history[i];
      int last_card = -1;
      for (auto card : cards) {
        if (card - last_card > 1) {
          for (int j = 1; j < card - last_card; ++j) {
            int skipped_card = 4*i + last_card + j;
            if (remain_cards.count(skipped_card) == 1) {
              prev_predict.push_back(skipped_card);
              remain_cards.erase(skipped_card);
            }
          }
          last_card = card;
        }
      }
    }
  } else if (my_pos == 1) {
    // prev is dizhu, next is menban
    for (auto cards : prev_history) {
      int last_card = -1;
      for (auto card : cards) {
        if (card > last_card) {
          last_card = card;
        } else {
          enemy_is_ordered = false;
          break;
        }
      }
    }
    if (enemy_is_ordered) {
      for (auto cards : next_history) {
        int last_card = -1;
        for (auto card : cards) {
          if (card > last_card) {
            last_card = card;
          } else {
            enemy_is_ordered = false;
            break;
          }
        }
      }
    }
    if (!enemy_is_ordered) {
      return;
    }
    for (int i = 0; i < 13; ++i) {
      auto cards = prev_history[i];
      int last_card = -1;
      for (auto card : cards) {
        if (card - last_card > 1) {
          for (int j = 1; j < card - last_card; ++j) {
            int skipped_card = 4*i + last_card + j;
            if (remain_cards.count(skipped_card) == 1) {
              next_predict.push_back(skipped_card);
              remain_cards.erase(skipped_card);
            }
          }
          last_card = card;
        }
      }
    }
    for (int i = 0; i < 13; ++i) {
      auto cards = next_history[i];
      int last_card = -1;
      for (auto card : cards) {
        if (card - last_card > 1) {
          for (int j = 1; j < card - last_card; ++j) {
            int skipped_card = 4*i + last_card + j;
            if (remain_cards.count(skipped_card) == 1) {
              prev_predict.push_back(skipped_card);
              remain_cards.erase(skipped_card);
            }
          }
          last_card = card;
        }
      }
    }
  } else {
    // prev is xiajia, next is dizhu
    for (auto cards : prev_history) {
      int last_card = -1;
      for (auto card : cards) {
        if (card > last_card) {
          last_card = card;
        } else {
          enemy_is_ordered = false;
          break;
        }
      }
    }
    if (enemy_is_ordered) {
      for (auto cards : next_history) {
        int last_card = -1;
        for (auto card : cards) {
          if (card > last_card) {
            last_card = card;
          } else {
            enemy_is_ordered = false;
            break;
          }
        }
      }
    }
    if (!enemy_is_ordered) {
      return;
    }
    for (int i = 0; i < 13; ++i) {
      auto cards = prev_history[i];
      int last_card = -1;
      for (auto card : cards) {
        if (card - last_card > 1) {
          for (int j = 1; j < card - last_card; ++j) {
            int skipped_card = 4*i + last_card + j;
            if (remain_cards.count(skipped_card) == 1) {
              next_predict.push_back(skipped_card);
              remain_cards.erase(skipped_card);
            }
          }
          last_card = card;
        }
      }
    }
    for (int i = 0; i < 13; ++i) {
      auto cards = next_history[i];
      int last_card = -1;
      for (auto card : cards) {
        if (card - last_card > 1) {
          for (int j = 1; j < card - last_card; ++j) {
            int skipped_card = 4*i + last_card + j;
            if (remain_cards.count(skipped_card) == 1) {
              prev_predict.push_back(skipped_card);
              remain_cards.erase(skipped_card);
            }
          }
          last_card = card;
        }
      }
    }
  }
}

/*****************************************************************************/
// Main Program: Get Input, Get Recent Game Status, Start Search and Output
/*****************************************************************************/
int main() {
  std::srand(unsigned(std::time(0)));
  input();
  get_predict();

  /** Below for debug **/
  //cout << "prev:";
  //for (int i = 0; i < 15; ++i) {
  //  for (auto card : prev_history[i]) {
  //    cout << card << ' ';
  //  }
  //  cout << "|";
  //}
  //cout << endl;
  //cout << "next:";
  //for (int i = 0; i < 15; ++i) {
  //  for (auto card : next_history[i]) {
  //    cout << card << ' ';
  //  }
  //  cout << "|";
  //}
  //cout << endl;
  //cout << "rem:";
  //for (auto card : remain_cards) {
  //  cout << card << ' ';
  //}
  //cout << endl;
  debug_buffer << "prev pred:";
  for (auto card : prev_predict) {
    debug_buffer << card << ',';
  }
  debug_buffer << " | ";
  debug_buffer << "next pred:";
  for (auto card : next_predict) {
    debug_buffer << card << ',';
  }
  debug_buffer << " | ";
  /** Above for debug **/

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

  output(mc_play(last_hand, prev_pass));
}
