#include <iostream>
#include <string>
#include <vector>

namespace constants {
static const std::string SPADE = "\u2660";
static const std::string HEARTS = "\u2661";
static const std::string DIAMONDS = "\u2662";
static const std::string CLUBS = "\u2663";
static const std::string suits[] = {SPADE, HEARTS, DIAMONDS, CLUBS};
static const std::string cards[] = {"A", "2", "3",  "4", "5", "6", "7",
                                    "8", "9", "10", "J", "Q", "K"};
static const unsigned int N_DECK = 6;
static const std::string DEALER_NAME = "Dealer";
}  // namespace constants

//
// Card Class
//
struct Card {
  std::string suit;
  unsigned int value;
  std::string printable;

  Card() = default;
  Card(const std::string t_suit, unsigned int t_val,
       const std::string t_printable)
      : suit(t_suit), value(t_val), printable(t_printable){};
  ~Card() = default;

  Card(const Card& rhs)
      : suit(rhs.suit), value(rhs.value), printable(rhs.printable) {
    std::cout << "Card Copy Constructed \n";
  }
  Card& operator=(const Card& rhs) {
    if (this != &rhs) {
      suit = rhs.suit;
      value = rhs.value;
      printable = rhs.printable;
      std::cout << "Card Copy Assigned \n";
      return *this;
    }
  }
  // Card& operator= (const Card& rhs) = default;

  Card& operator=(Card&& rhs) noexcept = default;
  Card(Card&& rhs) noexcept = default;
  // Card& operator= (Card&& rhs) noexcept {
  //     if(this != &rhs) {
  //         suit = rhs.suit);
  //         value = rhs.value);
  //         printable = rhs.printable);

  //         rhs.suit.clear();
  //         rhs.printable.clear();
  //         rhs.value = 0;
  //     }
  //     return *this;
  // }
  // Card(Card&& rhs) noexcept :
  //     suit(rhs.suit)),
  //     value(rhs.value)),
  //     printable(rhs.printable)) {
  //         rhs.suit.clear();
  //         rhs.printable.clear();
  //         rhs.value = 0;
  // }

  friend std::ostream& operator<<(std::ostream& os, const Card& card) {
    os << card.printable << " [" << card.value << "]";
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
    static_assert(N >= 1, "Deck needs to be initialized with N <= 1");
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
      std::cout << card << "\t";
      i++;
      if (i == 14) {
        std::cout << '\n';
        i = 0;
      }
    }
    std::cout << '\n';
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
  std::string m_name;
  int val = 0;
  Deck<constants::N_DECK>& m_deck = g_deck;
  const int id;

 public:
  Player(const std::string& t_name, int t_bal = 5000, int t_id = -1)
      : m_name(t_name), balance(t_bal), id(t_id) {}

  void reset_hand() { m_hand.clear(); };

  inline bool has_busted() { return m_busted; }

  inline const std::string& get_name() const { return m_name; }

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
  std::cout << "--------------- Players ---------------\n";
  static bool first = true;
  static bool show_player;
  if (!show_values) {
    std::cout << dealer.get_name() << "'s hand: \n";
    for (auto& card : dealer.get_hand()) {
      if (first) {
        std::cout << "\t* [Unknown]\n";
        first = false;
        show_player = true;
        continue;
      } else {
        std::cout << '\t' << card << " \n";
      }
    }
  } else {
    std::cout << dealer.get_name() << "'s hand: \n";
    for (auto& card : dealer.get_hand()) {
      std::cout << '\t' << card << " \n";
    }
  }
  if (show_values) {
    std::cout << "\tValue: " << dealer.get_val() << '\n';
  }

  std::cout << player.get_name() << "'s hand: [Balance: $"
            << player.get_balance() << "]\n";
  for (auto& card : player.get_hand()) {
    std::cout << '\t' << card << " \n";
  }
  if (show_values || show_player) {
    std::cout << "\tValue: " << player.get_val() << '\n';
  }

  std::cout << "-------------------------------------\n ";
  std::cout << "\n";
}

//
//  Print the hand of a single player
//
void print_hand(Player& player) {
  if (player.get_name() == constants::DEALER_NAME) {
    std::cout << player.get_name() << "'s hand: \n";

  } else {
    std::cout << "Your hand: [Balance: " << player.get_balance() << "]\n";
  }
  for (auto& card : player.get_hand()) {
    std::cout << '\t' << card << " \n";
  }
  std::cout << "\tValue: " << player.get_val() << '\n';
  std::cout << "\n";
}

//
// Create a player from a string(name), and int(balance)
//
Player create_player(const std::string& name, int balance = 5000) {
  static int id = 0;
  return {name, balance, id};
};

void play_round(Player dealer, std::vector<int>& player_ids);
void play_round(Player& dealer, Player& player);

#include <thread>

//
// Main Function
//
int main(int argc, char const* argv[]) {
  // Deck<1> deck;
  Player dealer =
      create_player(constants::DEALER_NAME, std::numeric_limits<int>::max());

  std::cout << "Welcome to the Blackjack table!\nYour dealer today is "
            << constants::DEALER_NAME << "\nWhat's your name? \n";
  std::string name = "Kenny";
  std::cin >> name;

  Player player = create_player(name);

  // Player needs to buy in before dealt cards
  std::cout << "Buy in ===============================\n";
  play_round(dealer, player);

  std::string answer;
  while (answer != "y" || answer != "n" || answer != "yes" || answer != "no") {
    std::cout << "Would you like to keep playing, yes(s) or no(n)? ";
    std::cin >> answer;
    if (answer == "y" || answer == "yes") {
      if (player.get_balance() >= 500) {
        dealer.reset_hand();
        player.reset_hand();
        play_round(dealer, player);
      } else
        std::cout << "You don't have enough to keep playing. Your balance:"
                  << player.get_balance() << "Come again soon! Goodbye!\n";
    } else if (answer == "n" || answer == "no") {
      std::cout << "Come again soon! Goodbye!\n";
      break;
    } else {
      std::cout << "Player enter a valid input!\n";
    }
  }
  return 0;
}

void clear_console() {
  pause();
  system("clear");
}

void pause() {
  std::cout << "Press enter to continue...";
  std::cin.ignore();
  std::cout << '\n';
}

void play_round(Player& dealer, Player& player) {
  dealer.get_card();
  dealer.get_card();

  player.get_card();
  player.get_card();

  print_hands(dealer, player, false);
  std::cout << "====================== Place bet ======================\n";

  int bet = 000;
  while (bet < 500) {
    std::cout << "How much would you like to bet?(At least $500): $";
    std::cin >> bet;
    if (bet == 0) {
      break;
    } else if (player.get_balance() < bet) {
      std::cout << "Your balance is too low to bet that amount\nYour balance: $"
                << player.get_balance() << '\n';
      bet = 0;
    } else if (bet < 500) {
      std::cout << "You've bet " << bet << ". You need to bet at least $500.\n";
    }
  }
  if (bet == 0) {
    std::cout << "End of round, you folded!\n";
    return;  // Game is over
  } else {
    player.take(bet);
    std::cout << "====================== Bets Placed ======================\n";

    // Dealer's turn
    // ==========================================================================
    print_hands(dealer, player);
    std::cout << "============= " << dealer.get_name()
              << "'s Turn =============\n";
    if (dealer.get_val() > 17) {
      std::cout << dealer.get_name() << " stays\n";
      // pause();
    }
    while (dealer.get_val() <= 17) {
      std::cout << '\n' << dealer.get_name() << " hits! \n";
      dealer.get_card();
      std::cout << dealer.get_name()
                << " Received: " << dealer.get_hand().back() << '\n';
      // print_hand(dealer);
      if (dealer.has_busted()) {
        break;
      } else if (dealer.get_val() > 17) {
        std::cout << dealer.get_name() << " stays\n";
        // pause();
        break;
      }
      // else {
      // pause();
      // }
    }
    print_hand(dealer);

    // Player's turn
    // ==========================================================================
    if (!dealer.has_busted()) {
      print_hands(dealer, player);
      std::string answer;

      std::cout << "============= " << player.get_name()
                << "'s Turn =============\n";

      while (answer != "h" || answer != "s" || answer != "hit" ||
             answer != "stay") {
        print_hand(player);
        std::cout << "Would  you like to hit(h) or stay(s)? ";
        std::cin >> answer;
        if (answer == "h" || answer == "hit") {
          player.get_card();
          std::cout << player.get_name()
                    << " Received: " << player.get_hand().back() << "\n\n";
          if (player.has_busted()) {
            std::cout << "You busted!\n";
            print_hand(player);
            break;
          }
        } else if (answer == "s" || answer == "stay") {
          std::cout << "Player's turn is over.\n";
          break;
        } else {
          std::cout << "Player enter a valid input!\n";
        }
      }
    }

    clear_console();
    print_hands(dealer, player);
    if (player.has_busted()) {
      std::cout << "You busted!\n";
    } else if (dealer.has_busted()) {
      std::cout << dealer.get_name() << " busted!\nYou win!\n";
      std::cout << "You've won $" << bet * 2 << '\n';
      player.give(bet * 2);
    } else if (dealer.get_val() > player.get_val()) {
      std::cout << dealer.get_name() << " had the higher hand\nYou lose.\n";
    } else if (dealer.get_val() < player.get_val()) {
      std::cout << "Congrats! You had the higher hand\nYou win!\n";
      player.give(bet * 2);
      std::cout << "You've won $" << bet * 2 << '\n';
    } else {
      std::cout << "It's a tie!\n";
    }
  }
}

/*
  Fix Aces, they should have a variable value, either 11 or 1 when player busts
 */