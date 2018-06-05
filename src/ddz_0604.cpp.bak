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
  {0.97749,1.0224,0.97953,0.92894,0.87765,0.81077,0.72834,0.61897,0.5092,0.41047,0.31054,0.22855,0.16265,0.11,0.070436,0.044653,0.020267,0.0039768,-0.0028577},
  {0.16648,0.11331,0.036831,0.011483,0.024384,0.037259,0.044231,0.026411,0.014226,0.043581,0.027702,0.035979,0.028049,0.023299,0.0078892,-0.023719,-0.053968,-0.039704,-0.050196},
  {-0.11597,-0.10861,-0.10629,-0.064259,0.01572,0.062089,0.083938,0.096102,0.13379,0.12582,0.077063,0.092886,0.054001,0.067814,0.079029,0.07136,0.041336,0.031354,0.0036507},
  {-0.069244,-0.066453,-0.047251,-0.030637,0.010007,0.04619,0.095265,0.093101,0.099193,0.073094,0.05959,0.037812,0.054697,0.034725,0.0016468,-0.022418,-0.024485,-0.010748,-0.013712},
  {-0.16077,-0.1316,-0.097927,-0.034679,0.015109,0.064265,0.098691,0.09103,0.068282,0.077041,0.05068,0.037244,0.045711,0.0532,0.028645,0.0044036,0.0052963,-0.0052343,-0.00082461},
  {-0.30506,-0.22818,-0.16658,-0.10436,-0.036781,0.043248,0.097495,0.12478,0.1128,0.1114,0.11019,0.080913,0.049425,0.0567,0.046851,-0.0040509,-0.027035,-0.064769,-0.064507},
  {-0.36006,-0.28337,-0.22691,-0.14883,-0.082345,-0.020652,0.011917,0.074713,0.0935,0.078233,0.067516,0.091858,0.095521,0.057019,0.026953,0.030074,0.014243,-0.016474,-0.033106},
  {-0.36617,-0.2503,-0.17505,-0.1056,-0.054348,-0.020379,0.0047701,0.029083,0.02277,0.031469,0.0046484,0.0046719,-0.010233,-0.0092284,0.00035484,0.0025596,-0.020415,-0.043027,-0.04864},
  {-0.22589,-0.17596,-0.16246,-0.089864,-0.0098885,0.03405,0.04799,0.043004,0.04004,0.035072,0.038557,0.04392,0.034445,0.027159,0.018124,0.013557,0.0063895,-0.003632,-0.0018672},
  {-0.042526,-0.027363,0.02538,0.034156,0.071734,0.08258,0.092563,0.081922,0.077403,0.080988,0.087304,0.054578,0.082681,0.085828,0.064787,0.097323,0.077606,0.026813,0.008849},
  {0.071614,0.047377,0.0083923,0.02835,0.046949,0.073608,0.1171,0.14472,0.13333,0.12311,0.10615,0.091251,0.082248,0.0821,0.043703,0.039106,0.029493,0.043579,0.037506},
  {0.30356,0.18125,0.17698,0.18948,0.18755,0.17787,0.17275,0.16636,0.1893,0.17196,0.17166,0.16019,0.16651,0.18352,0.16382,0.17179,0.14079,0.14771,0.14608},
  {0.46773,0.46532,0.5059,0.44404,0.39019,0.37642,0.37917,0.39012,0.39338,0.39366,0.3775,0.38723,0.39648,0.40741,0.39398,0.44068,0.45615,0.47488,0.41529},
  {1.0103,1.0466,1.0724,1.0787,1.0605,1.0413,1.0095,0.97167,0.93383,0.90118,0.85666,0.80723,0.77354,0.73304,0.74865,0.75095,0.79143,0.77138,0.77206},
  {1.2526,1.28,1.236,1.1408,1.0536,1.0088,0.96085,0.94005,0.88439,0.81013,0.77737,0.74849,0.6807,0.63031,0.55958,0.48912,0.41816,0.39193,0.39046},
  {0.2464,0.27269,0.32353,0.39549,0.48346,0.53329,0.48258,0.41878,0.36902,0.31604,0.29461,0.23034,0.18249,0.16082,0.12629,0.13496,0.13529,0.080648,0.070999},
  {0.10022,0.11587,0.14678,0.19194,0.24951,0.31656,0.36749,0.34321,0.30675,0.26765,0.19636,0.15413,0.086474,0.060529,0.06431,0.048698,0.056074,0.036023,0.057043},
  {0.033407,0.041891,0.059624,0.087963,0.1285,0.18257,0.24847,0.33244,0.37953,0.41574,0.41731,0.43936,0.41815,0.37139,0.35697,0.33913,0.33047,0.33732,0.33734},
  {-0.61311,-0.66149,-0.61514,-0.59904,-0.56265,-0.55627,-0.53572,-0.5109,-0.48529,-0.4548,-0.42094,-0.38636,-0.35699,-0.34395,-0.31031,-0.28866,-0.25891,-0.24025,-0.22821},
  {0.22929,0.2538,0.3012,0.36342,0.42572,0.46464,0.47423,0.48136,0.4721,0.46248,0.46302,0.43894,0.41125,0.4113,0.42923,0.42709,0.42295,0.40804,0.4236},
  {-0.23241,0.12877,0.32592,0.38469,0.4196,0.47078,0.48308,0.4726,0.47382,0.46247,0.4501,0.4513,0.44964,0.42653,0.37777,0.35255,0.37448,0.37665,0.38333},
  {-0.10948,-0.12162,-0.06903,-0.090218,-0.056757,-0.0033858,0.039054,-0.010949,-0.017442,-0.024058,-0.0090817,0.0028256,0.00022925,-0.0078394,0.018337,0.0015889,0.01355,0.013311,-0.0098362},
  {0.14195,0.14426,0.14737,0.1628,0.19246,0.17547,0.11809,0.09731,0.072795,0.088021,0.06784,0.052748,0.017395,0.033722,0.0085069,0.020406,-0.01546,0.0031825,0.0096397}
},{
  {0.61927,0.46098,0.37176,0.30306,0.25184,0.20107,0.15953,0.11702,0.085353,0.053417,0.030614,0.010538,-0.0018644,-0.01247,-0.025019,-0.028484,-0.026694,-0.01834,-0.014213},
  {0.12356,0.11067,0.080584,0.056919,0.034888,0.038967,0.02301,0.013763,0.0035183,0.0010561,-0.0065563,-0.0064996,-0.0038102,0.0021399,-0.0061516,-0.014033,-0.018053,-0.018001,-0.017486},
  {0.10153,0.088664,0.090866,0.092332,0.086131,0.071502,0.045114,0.033881,0.018223,0.0042232,0.011393,0.01799,-0.00123,0.004904,-0.0072709,-0.0097101,-0.0093327,-0.010216,-0.010327},
  {0.047754,0.025963,0.020807,0.026785,0.011515,0.027627,0.044883,0.019697,0.015416,-0.0091379,-0.010305,-0.012373,-0.017102,-0.017279,-0.014725,-0.016333,-0.011218,-0.011664,-0.011522},
  {0.021571,0.01283,0.0045441,0.0079649,0.023958,0.042431,0.047216,0.040062,0.035009,0.020948,0.0043522,-0.00033482,-0.0099519,-0.02445,-0.020589,-0.014374,-0.022723,-0.020758,-0.019238},
  {-0.014475,-0.013183,-0.015296,-0.0097251,-0.0066448,0.0059895,0.041568,0.043757,0.046587,0.032825,0.0081232,-0.0013252,-0.0017874,0.0077539,-0.0015249,-0.013226,-0.00746,-0.008306,-0.0084414},
  {-0.034906,-0.033132,-0.0041726,-0.011174,-0.0069965,-0.0072332,-0.014544,-0.011069,-0.0033959,-0.0080474,-0.013809,-0.0073703,-0.025013,-0.036291,-0.032405,-0.040638,-0.029571,-0.026991,-0.025029},
  {-0.051169,-0.062814,-0.057887,-0.044533,-0.01175,-0.019715,-0.021812,-0.035147,-0.018125,-0.015979,-0.024611,-0.026252,-0.028169,-0.043714,-0.03572,-0.033121,-0.030469,-0.027473,-0.025302},
  {-0.052429,-0.057578,-0.067062,-0.052645,-0.037624,-0.028586,-0.027162,-0.026129,-0.020333,-0.014041,0.0079992,-0.0047064,-0.00087364,0.0089572,-0.010792,-0.011315,0.00010798,-0.002224,-0.0032182},
  {-0.038217,-0.028338,-0.030053,-0.023702,0.0033196,0.0073739,0.025935,0.026047,0.0059713,-0.013325,-0.0052385,-0.010769,0.0013123,-0.0088509,-0.001683,-0.0016519,-0.0007557,-0.0033775,-0.0044951},
  {-0.064335,-0.084737,-0.085193,-0.05512,-0.029708,-0.023322,-0.0065074,-0.018089,-0.016867,0.0015317,-0.0062679,0.0019552,0.019243,0.028864,0.018465,0.019811,0.018379,0.012221,0.0090264},
  {-0.006933,-0.027525,-0.013514,-0.016047,-0.0057901,0.018264,0.02235,0.032867,0.033654,0.035059,0.041391,0.038201,0.069058,0.065007,0.060395,0.050593,0.054561,0.042588,0.035868},
  {0.1774,0.14879,0.14698,0.14196,0.089753,0.080914,0.086581,0.09651,0.10797,0.12227,0.12667,0.11797,0.11933,0.11498,0.12151,0.11697,0.099852,0.080098,0.068713},
  {0.2724,0.26785,0.25781,0.24595,0.22999,0.20512,0.20383,0.19778,0.19204,0.18873,0.17836,0.16661,0.16435,0.16773,0.16445,0.157,0.13702,0.10398,0.085779},
  {0.24229,0.28435,0.32143,0.32356,0.32356,0.31675,0.29828,0.27706,0.26157,0.25548,0.24904,0.23551,0.22051,0.21035,0.20095,0.17913,0.14931,0.10769,0.08606},
  {0.041625,0.061062,0.10133,0.1633,0.24468,0.27153,0.20449,0.16493,0.12785,0.090749,0.060135,0.04266,0.037448,0.028287,0.019511,0.013437,0.0099992,0.0072923,0.0058089},
  {0.0029153,0.0051279,0.010472,0.02074,0.038361,0.065809,0.092921,0.098559,0.094344,0.072025,0.050441,0.018692,0.021198,-0.0060323,-0.0034033,-0.0070302,-0.010314,-0.0091876,-0.0084796},
  {0.00022449,0.0004057,0.00086354,0.0018078,0.0035911,0.006727,0.010605,0.017227,0.024992,0.021013,0.018221,0.01474,0.015174,0.020159,0.029536,0.020484,0.020408,0.015623,0.012856},
  {0.34159,0.098531,0.013149,-0.03082,-0.031662,-0.045205,-0.044259,-0.039397,-0.040855,-0.035186,-0.036651,-0.032488,-0.03056,-0.020148,-0.027122,-0.022281,-0.032212,-0.023169,-0.018342},
  {0.031522,0.042878,0.063258,0.088896,0.11147,0.13026,0.13483,0.14594,0.16146,0.17192,0.18374,0.17987,0.17683,0.16417,0.14674,0.12322,0.11611,0.087761,0.07234},
  {0.070239,0.12703,0.15542,0.13762,0.12801,0.15172,0.1482,0.1421,0.12314,0.095743,0.091396,0.076942,0.059321,0.036639,0.047812,0.037557,0.067892,0.067058,0.06438},
  {0.18286,0.20193,0.18834,0.13308,0.11188,0.069765,0.056491,0.042234,0.027221,0.01743,0.018669,0.018501,0.016585,0.00023493,0.012943,0.0092746,0.010569,0.0090336,0.0081176},
  {0.05924,0.078995,0.1172,0.16201,0.17595,0.13856,0.080228,0.073313,0.073709,0.061319,0.05296,0.053537,0.035849,0.028244,0.027818,0.027141,0.013883,0.012479,0.011521}
},{
  {0.83934,0.59786,0.4881,0.41367,0.34724,0.28016,0.22527,0.16643,0.12014,0.078912,0.051597,0.033048,0.02308,0.017371,0.011136,0.0065463,0.0036149,0.0014223,0.00044481},
  {-0.12213,-0.1264,-0.10751,-0.041778,-0.020169,0.019058,0.015163,0.013057,0.017281,0.0063743,0.011094,-0.0020858,-0.017008,-0.0044602,-0.011677,-0.011732,-0.013815,-0.013469,-0.013027},
  {-0.082504,-0.089475,-0.083218,-0.052436,-0.028006,0.002577,0.008785,0.013841,0.016286,0.0047277,-0.00087587,-0.0034584,-0.0073066,-0.0012617,-0.016106,-0.014604,-0.016191,-0.015193,-0.014391},
  {-0.11719,-0.12328,-0.11167,-0.077893,-0.040984,-0.015721,-0.0062203,0.00041884,0.0029105,-0.016635,-0.02277,-0.020265,-0.031375,-0.029197,-0.016069,-0.0089911,-0.010617,-0.010165,-0.0097281},
  {-0.15914,-0.16074,-0.11935,-0.076691,-0.029339,-0.020456,0.0066311,0.017775,0.027751,0.01989,-0.0077441,-0.018926,-0.022122,-0.022654,-0.024057,-0.014692,-0.0019956,-0.0023184,-0.0023991},
  {-0.16541,-0.16753,-0.12905,-0.087191,-0.04087,-0.027672,-0.0027791,-0.0092287,-0.0088376,-0.016848,-0.025781,-0.037991,-0.044863,-0.024215,-0.0254,-0.031756,-0.023536,-0.021403,-0.019944},
  {-0.072523,-0.082483,-0.076173,-0.051554,-0.023126,0.00016344,-0.003928,0.0040024,0.011573,0.0018836,-0.017552,-0.016905,-0.015594,-0.011141,-0.0058483,-0.0049629,0.00185,0.00058009,-1.8507e-05},
  {-0.025001,-0.057528,-0.056894,-0.034829,-0.020343,0.00031012,-0.0077479,-0.0062768,-0.021497,-0.0161,-0.017079,-0.022775,-0.01611,0.0016713,0.00094066,-0.0032554,-0.00092929,-0.0017019,-0.0020184},
  {0.010501,-0.0011359,0.015872,0.023966,0.015703,0.021339,0.018902,0.021035,0.018377,0.014482,0.005614,0.0033782,0.0021397,-0.01467,-0.0017268,-0.0059558,-8.1768e-05,-0.0013539,-0.0019112},
  {0.024455,-0.0031729,0.0041655,0.015681,0.021178,0.025665,0.020428,0.02194,0.015139,0.016529,0.018282,0.0063823,0.018568,0.0077347,0.0040221,0.0080837,0.0025388,0.00082006,-1.2975e-06},
  {0.20042,0.11606,0.092878,0.06572,0.049236,0.025848,0.034266,0.020192,0.030675,0.027721,0.025106,0.020822,0.018804,0.012013,0.018679,0.011739,0.015247,0.011394,0.0093452},
  {0.25583,0.19737,0.17653,0.14466,0.13718,0.12545,0.12726,0.12392,0.1275,0.11272,0.10062,0.090139,0.089925,0.074158,0.043296,0.028323,0.027038,0.021324,0.018178},
  {0.43329,0.42472,0.4182,0.36128,0.29559,0.24317,0.21615,0.1863,0.16705,0.14068,0.13867,0.1138,0.1168,0.094187,0.073989,0.062342,0.036312,0.028942,0.024849},
  {0.48772,0.50622,0.49226,0.46264,0.40262,0.35215,0.30999,0.28695,0.24368,0.21893,0.20261,0.17945,0.15442,0.13091,0.10607,0.088396,0.064594,0.049928,0.041939},
  {0.40352,0.44744,0.44205,0.40153,0.34935,0.3073,0.27508,0.22404,0.17208,0.12569,0.10428,0.088043,0.090992,0.081838,0.067462,0.045231,0.012111,0.007691,0.0057298},
  {0.057606,0.079395,0.12352,0.1896,0.27434,0.28902,0.21847,0.12813,0.086821,0.069994,0.059226,0.046136,0.035809,0.024355,0.014237,0.0055091,0.0079016,0.0061344,0.0051655},
  {0.0051617,0.0085013,0.01626,0.030417,0.053369,0.087024,0.12475,0.13656,0.11609,0.067347,0.038485,0.02865,0.023907,0.011516,0.015279,0.0060576,0.0080757,0.006958,0.0062649},
  {-1.2346e-05,-2.4398e-05,-5.6746e-05,-0.00012888,-0.00027749,-0.00056514,-0.0010909,-0.0020011,-0.0034966,-0.0042113,-0.0032236,-0.0084909,-0.015503,-0.017609,-0.011694,-0.002463,0.0099062,0.0080117,0.0069042},
  {0.32981,-0.023505,-0.11133,-0.12057,-0.10485,-0.11029,-0.097754,-0.09448,-0.081867,-0.067818,-0.051166,-0.041815,-0.040609,-0.034145,-0.032836,-0.024133,-0.03445,-0.027723,-0.024031},
  {0.058401,0.073161,0.10113,0.13326,0.15324,0.15535,0.14778,0.14985,0.155,0.16133,0.1695,0.16342,0.15726,0.13672,0.11492,0.089576,0.084433,0.06354,0.05257},
  {0.3553,0.40053,0.38977,0.33345,0.25957,0.22618,0.2034,0.18652,0.16926,0.12306,0.098977,0.0747,0.063458,0.020392,0.031658,0.038682,0.042677,0.038864,0.035992},
  {0.10798,0.10718,0.071362,0.051179,0.03626,0.0344,0.01932,0.013708,0.019354,0.0087751,0.010475,0.01043,0.009582,-0.008865,0.0063102,-0.00036528,0.018488,0.016217,0.014839},
  {0.051028,0.062806,0.084692,0.10412,0.10528,0.067177,0.037268,0.015891,0.010684,0.0054029,0.00619,0.0090124,0.0075595,0.0018934,0.0021615,0.0015317,0.0070589,0.0069154,0.0067282},
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
  if (remain_number == 0) {
    return 10000.;
  }
  if (remain_number == 20) {
    return -10000.;
  }
  for (int i = 0; i < features.size(); ++i) {
    value += features[i] * weight[pos][i][remain_number-1];
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
Hand mc_step(int pos, Hand prev, int random_number, int pass_number, int search_index) {
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
  //if ((pos == 2 && pass_number == 0) || (pos == 1 && pass_number == 1)) {
  //  Hand pass_hand = Hand();
  //  if (check_valid(pass_hand, prev)) {
  //    mc_redo(pos, pass_hand);
  //    double val = mc_evaluate(pos);
  //    hands.push_back(make_pair(pass_hand, val));
  //    mc_undo(pos, pass_hand);
  //  }
  //}
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
    if (search_index > 0) {
      rand_result = search_index;
    }
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
int mc_run(Hand prev_hand, Hand prev_prev_hand, bool prev_pass, int search_index) {

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
    Hand hand = mc_step(position, prev_hand, random_number, pass, search_index % 2);
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

    search_index /= 2;
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
          int search_times = 1;
          if (my_pos == 0) {
            search_times = 4;
          }
          int min_score = 10000.;
          for (int s = 0; s < search_times; ++s) {
            mc_init(cards);
            mc_calc_detail(my_pos, hand);
            int win_pos = mc_run(hand, prev, prev_pass, s);
            //cout << hand.width << endl;
            //cout << "score : " << score[0] << ',' << score[1] << ',' << score[2] << endl;
            int this_score = 0;
            if (my_pos == 0) {
              int delta = 2*score[0] - (score[1]+score[2]);
              this_score += delta;
              if (win_pos == 0) {
                this_score += 400;
              } else {
                this_score -= 400;
              }
            } else if (my_pos != 0) {
              int delta = (score[1]+score[2]) - 2*score[0];
              this_score += delta;
              if (win_pos == 0) {
                this_score -= 400;
              } else {
                this_score += 400;
              }
            }
            if (this_score < min_score) {
              min_score = this_score;
            }
          }
          it->second += min_score;
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
