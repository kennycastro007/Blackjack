#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <vector>
using namespace std::literals;

#ifdef WIN32
#include <fcntl.h>
#include <io.h>

static auto& COUT = std::wcout;
static auto& CIN = std::wcin;

using STRING = std::wstring;
#define _INIT _setmode(_fileno(stdout), 0x00020000);  // _O_U16TEXT
STRING operator"" _s(const char* string, std::size_t len) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
  std::wstring wide = converter.from_bytes(string);
  return wide;
}

void pause() {
  COUT << "Press enter to continue..."_s;
  CIN.ignore();
  COUT << '\n';
}
void clear_console() {
  pause();
  system("cls");
}
#else
#define _INIT
static auto& COUT = std::cout;
static auto& CIN = std::cin;
using STRING = std::string;

STRING operator"" _s(const char* string, std::size_t len) { return string; }

void pause() {
  COUT << "Press enter to continue..."_s;
  CIN.ignore();
  COUT << '\n';
}
void clear_console() {
  pause();
  system("clear");
}
#endif

namespace constants {
static const STRING SPADE = "\u2660"_s;
static const STRING HEARTS = "\u2665"_s;
static const STRING DIAMONDS = "\u2666"_s;
static const STRING CLUBS = "\u2663"_s;
static const STRING suits[] = {SPADE, HEARTS, DIAMONDS, CLUBS};
static const STRING cards[] = {"A"_s, "2"_s, "3"_s,  "4"_s, "5"_s, "6"_s, "7"_s,
                               "8"_s, "9"_s, "10"_s, "J"_s, "Q"_s, "K"_s};
static const unsigned int N_DECK = 6;
static STRING DEALER_NAME = "Dealer"_s;
}  // namespace constants

// Card Class
struct Card {
  STRING suit;
  unsigned int value;
  STRING printable;

  Card() = default;
  Card(const STRING t_suit, unsigned int t_val, const STRING t_printable)
      : suit(t_suit), value(t_val), printable(t_printable){};
  ~Card() = default;

  Card(const Card& rhs)
      : suit(rhs.suit), value(rhs.value), printable(rhs.printable) {
    COUT << "Card Copy Constructed \n"_s;
  }
  Card& operator=(const Card& rhs) {
    if (this != &rhs) {
      suit = rhs.suit;
      value = rhs.value;
      printable = rhs.printable;
      COUT << "Card Copy Assigned \n"_s;
      return *this;
    }
  }
  // Card& operator= (const Card& rhs) = default;

  Card& operator=(Card&& rhs) noexcept = default;
  Card(Card&& rhs) noexcept = default;
  friend decltype(COUT)& operator<<(decltype(COUT)& os, const Card& card) {
    os << card.printable << " ["_s << card.value << "]"_s;
    return os;
  };
};

#include <algorithm>
#include <chrono>
using namespace std::chrono_literals;
#include <random>

//
// Deck Class
//
template <unsigned int N = 1>
class Deck {
 private:
  std::vector<Card> m_deck;

  void shuffle() {
    std::default_random_engine re(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::shuffle(m_deck.begin(), m_deck.end(), re);
  }

 public:
  Deck() {
    static_assert(N >= 1, "Deck needs to be initialized with N <= 1"_s);
    m_deck.reserve(52 * N);
    for (unsigned int i = 0; i < N; i++) {
      for (auto& suit : constants::suits) {
        i = 1;
        for (auto& card : constants::cards) {
          if (i == 1) {  // card is an Ace
            m_deck.push_back({suit, 11, (card + suit)});
          } else if (i > 10) {  // Card is a face
            m_deck.push_back({suit, 10, (card + suit)});
          } else {
            m_deck.push_back({suit, i, (card + suit)});
          }
          i++;
        }
      }
    }
    shuffle();
  }

  Card&& get_card() {
    Card& card = m_deck.back();
    m_deck.pop_back();
    return std::move(card);
  }

  void print() {
    int i = 0;
    for (auto& card : m_deck) {
      COUT << card << "\t";
      i++;
      if (i == 14) {
        COUT << '\n';
        i = 0;
      }
    }
    COUT << '\n';
  }

  void reshuffle() {
    for (unsigned int i = 0; i < N; i++) {
      for (auto& suit : constants::suits) {
        i = 1;
        for (auto& card : constants::cards) {
          if (i == 1) {  // card is an Ace
            m_deck.push_back({suit, 11, (card + suit)});
          } else if (i > 10) {  // Card is a face
            m_deck.push_back({suit, 10, (card + suit)});
          } else {
            m_deck.push_back({suit, i, (card + suit)});
          }
          i++;
        }
      }
    }
  };
};

Deck<constants::N_DECK> g_deck;

//
//  Player Class
//
class Player {
 private:
  bool m_busted = false;
  bool has_ace = false, ace_low = false, ace_high = false;
  int balance;  // US Dollars [$]
  std::vector<Card> m_hand;
  STRING m_name;
  int val = 0;
  Deck<constants::N_DECK>& m_deck = g_deck;
  const int id;

 public:
  Player(const STRING& t_name, int t_bal = 5000, int t_id = -1)
      : m_name(t_name), balance(t_bal), id(t_id) {}

  void reset_hand() { m_hand.clear(); };

  inline bool has_busted() { return m_busted; }

  inline const STRING& get_name() const { return m_name; }

  void get_card() {
    Card card = m_deck.get_card();
    int value;
    m_hand.emplace_back(card.suit, card.value, card.printable);
    // m_hand.emplace_back(card.suit, 11, card.printable);
    if (card.value == 0 || card.value == 11) {
      if (m_busted) {
        has_ace = true;
        ace_low = true;
        value = 1;
        card.value = 1;

      } else {
        has_ace = true;
        ace_high = true;
        value = 11;
        card.value = 11;
      }
    } else {
      value = card.value;
    }
    val += value;
    if (val > 21) {
      m_busted = true;
    }
  };

  const int& get_val() { return val; }

  const int& get_id() { return id; }

  const std::vector<Card>& get_hand() const { return m_hand; }

  const int& get_balance() const { return balance; }

  inline int take(const int& amount) {
    balance -= amount;
    return amount;
  }

  inline void give(const int& amount) { balance += amount; }
};

void clear_console();
void pause();

#include <cstdlib>
//
//  Print the hands of multiple players
//
void print_hands(Player& dealer, Player& player, bool show_values = true) {
  clear_console();
  COUT << "--------------- Players ---------------\n"_s;
  static bool first = true;
  static bool show_player;
  if (!show_values) {
    COUT << dealer.get_name() << "'s hand: \n"_s;
    for (auto& card : dealer.get_hand()) {
      if (first) {
        COUT << "\t* [Unknown]\n"_s;
        first = false;
        show_player = true;
        continue;
      } else {
        COUT << "\t"_s << card << " \n"_s;
      }
    }
  } else {
    COUT << dealer.get_name() << "'s hand: \n";
    for (auto& card : dealer.get_hand()) {
      COUT << "\t"_s << card << " \n"_s;
    }
  }
  if (show_values) {
    COUT << "\tValue: "_s << dealer.get_val() << "\n"_s;
  }

  COUT << player.get_name() << "'s hand: [Balance: $"_s << player.get_balance()
       << "]\n"_s;
  for (auto& card : player.get_hand()) {
    COUT << "\t"_s << card << " \n"_s;
  }
  if (show_values || show_player) {
    COUT << "\tValue: "_s << player.get_val() << "\n"_s;
  }

  COUT << "-------------------------------------\n "_s;
  COUT << "\n"_s;
}

//
//  Print the hand of a single player
//
void print_hand(Player& player) {
  if (player.get_name() == constants::DEALER_NAME) {
    COUT << player.get_name() << "'s hand: \n"_s;

  } else {
    COUT << "Your hand: [Balance: "_s << player.get_balance() << "]\n"_s;
  }
  for (auto& card : player.get_hand()) {
    COUT << "\t"_s << card << " \n"_s;
  }
  COUT << "\tValue: "_s << player.get_val() << "\n"_s;
  COUT << "\n"_s;
}

//
// Create a player from a string(name), and int(balance)
//
Player create_player(const STRING& name, int balance = 5000) {
  static int id = 0;
  return {name, balance, id};
};

void play_round(Player dealer, std::vector<int>& player_ids);
void play_round(Player& dealer, Player& player);

// #include <thread>

//
// Main Function
//
int main(int argc, char const* argv[]) {
  _INIT
  Player dealer =
      create_player(constants::DEALER_NAME, std::numeric_limits<int>::max());

  COUT << "Welcome to the Blackjack table!\nYour dealer today is "_s
       << constants::DEALER_NAME << "\nWhat's your name? \n"_s;
  STRING name = "Kenny"_s;
  CIN >> name;

  Player player = create_player(name);

  // Player needs to buy in before dealt cards
  COUT << "Buy in ===============================\n"_s;
  play_round(dealer, player);

  STRING answer;
  while (answer != "y"_s || answer != "n"_s || answer != "yes"_s ||
         answer != "no"_s) {
    COUT << "Would you like to keep playing, yes(s) or no(n)? "_s;
    CIN >> answer;
    if (answer == "y"_s || answer == "yes"_s) {
      if (player.get_balance() >= 500) {
        dealer.reset_hand();
        player.reset_hand();
        play_round(dealer, player);
      } else
        COUT << "You don't have enough to keep playing. Your balance:"_s
             << player.get_balance() << "Come again soon! Goodbye!\n"_s;
    } else if (answer == "n"_s || answer == "no"_s) {
      COUT << "Come again soon! Goodbye!\n"_s;
      break;
    } else {
      COUT << "Player enter a valid input!\n"_s;
    }
  }
  return 0;
}

void play_round(Player& dealer, Player& player) {
  dealer.get_card();
  dealer.get_card();

  player.get_card();
  player.get_card();

  print_hands(dealer, player, false);
  COUT << "====================== Place bet ======================\n"_s;

  int bet = 000;
  while (bet < 500) {
    COUT << "How much would you like to bet?(At least $500): $"_s;
    CIN >> bet;
    if (bet == 0) {
      break;
    } else if (player.get_balance() < bet) {
      COUT << "Your balance is too low to bet that amount\nYour balance: $"_s
           << player.get_balance() << '\n';
      bet = 0;
    } else if (bet < 500) {
      COUT << "You've bet " << bet << ". You need to bet at least $500.\n"_s;
    }
  }
  if (bet == 0) {
    COUT << "End of round, you folded!\n"_s;
    return;  // Game is over
  } else {
    player.take(bet);
    COUT << "====================== Bets Placed ======================\n"_s;

    // Dealer's turn
    //
    print_hands(dealer, player);
    COUT << "============= "_s << dealer.get_name()
         << "'s Turn =============\n "_s;
    if (dealer.get_val() > 17) {
      COUT << dealer.get_name() << " stays\n"_s;
      // pause();
    }
    while (dealer.get_val() <= 17) {
      COUT << '\n' << dealer.get_name() << " hits! \n"_s;
      dealer.get_card();
      COUT << dealer.get_name() << " Received: "_s << dealer.get_hand().back()
           << '\n';
      // print_hand(dealer);
      if (dealer.has_busted()) {
        break;
      } else if (dealer.get_val() > 17) {
        COUT << dealer.get_name() << " stays\n"_s;
        // pause();
        break;
      }
      // else {
      // pause();
      // }
    }
    print_hand(dealer);

    // Player's turn
    //
    if (!dealer.has_busted()) {
      print_hands(dealer, player);
      STRING answer;

      COUT << "============= "_s << player.get_name()
           << "'s Turn =============\n"_s;

      while (answer != "h"_s || answer != "s"_s || answer != "hit"_s ||
             answer != "stay"_s) {
        print_hand(player);
        COUT << "Would  you like to hit(h) or stay(s)? "_s;
        CIN >> answer;
        if (answer == "h"_s || answer == "hit"_s) {
          player.get_card();
          COUT << player.get_name() << " Received: "_s
               << player.get_hand().back() << "\n\n";
          if (player.has_busted()) {
            COUT << "You busted!\n"_s;
            print_hand(player);
            break;
          }
        } else if (answer == "s"_s || answer == "stay"_s) {
          COUT << "Player's turn is over.\n"_s;
          break;
        } else {
          COUT << "Player enter a valid input!\n"_s;
        }
      }
    }

    clear_console();
    print_hands(dealer, player);
    if (player.has_busted()) {
      COUT << "You busted!\n"_s;
    } else if (dealer.has_busted()) {
      COUT << dealer.get_name() << " busted!\nYou win!\n"_s;
      COUT << "You've won $"_s << bet * 2 << '\n';
      player.give(bet * 2);
    } else if (dealer.get_val() > player.get_val()) {
      COUT << dealer.get_name() << " had the higher hand\nYou lose.\n"_s;
    } else if (dealer.get_val() < player.get_val()) {
      COUT << "Congrats! You had the higher hand\nYou win!\n"_s;
      player.give(bet * 2);
      COUT << "You've won $"_s << bet * 2 << "\n"_s;
    } else {
      COUT << "It's a tie!\n"_s;
    }
  }
}

/*
  Fix Aces, they should have a variable value, either 11 or 1 when player
  busts
 */