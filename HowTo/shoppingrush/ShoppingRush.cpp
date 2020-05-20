/*
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * Example based on ideas from the excellent CPP training
 * "Praktikum Advanced Software Development with Modern C++" at LMU Munich,
 * Winter 2016
 */

/*
 * TASKS:
 * 1.  Understand control and data flow
 * 2.  Is your program free of data-races?
 * 2.1 check with DRace (see handout or slides)
 * 2.2 fix the concurrency issues and try again
 * 3.  Optimize locking pattern
 * 3.1 check with DRace
 * 4.  Think about the following: DRace reports 0 races => Program is race-free
 */

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <list>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#define CHECKOUT 0
#define TIMEOUT 1
#define PATIENCE 100
#define BUDGET 200

enum BigStoreProduct : int {
  Couch,
  FlatscreenTV,
  Hammock,
  CoffeeMachine,
  Ammunition
};

enum ToyStoreProduct : int {
  Pikachu,
  DoraTheExplorer,
  TrumpActionToy,
  Firetruck,
  TickleMeElmo
};

enum MarketProduct : int { Diesel, Fertilizer, Bleach, RubberGlove, Soap };

template <typename ProductT>
std::pair<ProductT, int> make() {
  return std::make_pair(static_cast<ProductT>(std::rand() % 5),
                        std::rand() % 100);
}

template <typename shop_t>
int shopping(std::timed_mutex& buy_mx, shop_t& shop,
             const std::string& shopname, int& budget, const int threadid) {
  bool checkout = false;

  while (!checkout) {
    // try to buy things, but only if shop is not empty

    size_t size = shop.size();
    if (size != 0) {
      auto elit = shop.begin();
      // find random product in shop
      std::advance(elit, (std::rand() % size));
      if (std::get<1>(*elit) < budget) {
        budget -= std::get<1>(*elit);

        // be careful, erase modifies the shop, hence lock here
        std::unique_lock<std::timed_mutex> buy_lock(buy_mx, std::defer_lock);

        if (!buy_lock.try_lock_for(std::chrono::milliseconds(PATIENCE))) {
          std::cout << "[T " << threadid << ", " << shopname << "] timeout"
                    << std::endl;
          return TIMEOUT;
        }
        // remove item from shop
        shop.erase(elit);
        std::cout << "[T " << threadid << ", " << shopname
                  << "] bought item, new budget " << budget << std::endl;
      } else {
        std::cout << "[T " << threadid << "] budget exhausted" << std::endl;
        checkout = true;
      }
    } else {
      std::cout << "[T " << threadid << "] no more products in this shop"
                << std::endl;
      checkout = true;
    }
  }
  return CHECKOUT;
}

template <typename BigStore_t, typename ToyStore_t, typename Market_t>
void go_wild(int threadid, BigStore_t& BigStore, ToyStore_t& ToyStore,
             Market_t& Market, int budget) {
  std::cout << "[T " << threadid << "] started" << std::endl;

  // ----------------------------------
  // TODO: how could that be optimized?
  // ----------------------------------
  static std::timed_mutex shop_mx;

  // reason why a customer returns from a shop
  int ret_reason = -1;
  while (ret_reason != CHECKOUT) {
    int shopnr = std::rand() % 3;
    switch (shopnr) {
      case 0:
        ret_reason = shopping(shop_mx, BigStore, "BigStore", budget, threadid);
        break;
      case 1:
        ret_reason = shopping(shop_mx, ToyStore, "ToyStore", budget, threadid);
        break;
      case 2:
        ret_reason = shopping(shop_mx, Market, "Market", budget, threadid);
    }
  }

  std::cout << "[T " << threadid << "] finished" << std::endl;
}

int main(int argc, char** argv) {
  std::cout << "Setup playground" << std::endl;

  static const int horde_size = 20;
  static const int BigStore_capacity = 100000;
  static const int ToyStore_capacity = 40000;
  static const int Market_capacity = 50000;

  // The shopping centers:
  std::multiset<std::pair<BigStoreProduct, int>> BigStore;
  std::multiset<std::pair<ToyStoreProduct, int>> ToyStore;
  std::multiset<std::pair<MarketProduct, int>> Market;

  // The dear customers:
  std::vector<std::thread> horde;

  // some tweaks
  horde.reserve(horde_size);

  std::generate_n(std::inserter(BigStore, BigStore.begin()), BigStore_capacity,
                  make<BigStoreProduct>);
  std::generate_n(std::inserter(ToyStore, ToyStore.begin()), ToyStore_capacity,
                  make<ToyStoreProduct>);
  std::generate_n(std::inserter(Market, Market.begin()), Market_capacity,
                  make<MarketProduct>);

  size_t num_total_products = BigStore.size() + ToyStore.size() + Market.size();
  std::cout << "Total number of products " << num_total_products << std::endl;

  for (int i = 0; i < horde_size; ++i) {
    horde.push_back(std::thread(
        go_wild<decltype(BigStore), decltype(ToyStore), decltype(Market)>, i,
        std::ref(BigStore), std::ref(ToyStore), std::ref(Market), BUDGET));
  }

  for (auto& t : horde) {
    t.join();
  }
  std::cout << "Black friday ended" << std::endl;
  return 0;
}
