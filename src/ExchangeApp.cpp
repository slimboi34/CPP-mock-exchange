#include "ExchangeApp.h"
#include <imgui.h>
#include <random>

namespace exchange {

ExchangeApp::ExchangeApp() {
  // Optionally seed the engine with some initial liquidity
  simulate_market_activity();
}

void ExchangeApp::simulate_market_activity() {
  // A simple uniform distribution to simulate organic order flow for visual
  // purposes
  static std::mt19937 gen(1337);
  static std::uniform_int_distribution<Price> price_dist(90, 110);
  static std::uniform_int_distribution<Quantity> qty_dist(10, 500);
  static std::uniform_int_distribution<int> side_dist(0, 1);

  // Add 10 random orders per frame
  for (int i = 0; i < 10; ++i) {
    Side s = (side_dist(gen) == 0) ? Side::Buy : Side::Sell;
    engine_.add_limit_order(next_order_id_++, s, price_dist(gen),
                            qty_dist(gen));
  }
}

void ExchangeApp::render() {
  // Simulate live orders arriving
  simulate_market_activity();

  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  ImGui::Begin("High Performance Mock Exchange (C++20)", nullptr,
               ImGuiWindowFlags_NoCollapse);

  ImGui::Text("Order Book Render - Nanosecond Latency Backend");
  ImGui::Separator();

  // Create a 2-column layout for Bids and Asks
  if (ImGui::BeginTable("OrderBookTable", 2,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("BIDS (Buyers)",
                            ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("ASKS (Sellers)",
                            ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    render_bids();

    ImGui::TableSetColumnIndex(1);
    render_asks();

    ImGui::EndTable();
  }

  ImGui::End();
}

void ExchangeApp::render_bids() {
  const auto &order_book = engine_.get_order_book();
  const auto &bids = order_book.get_bids();

  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green

  // Bids are already sorted descending (highest price first)
  int row_count = 0;
  for (const auto &[price, level] : bids) {
    if (row_count++ > 20)
      break; // Only show top 20 levels
    ImGui::Text("Px: %llu | Vol: %llu", price, level.total_volume);

    // Render individual orders at this price
    Order *curr = level.head;
    while (curr) {
      ImGui::BulletText("id:%llu (qty:%llu)", curr->id, curr->quantity);
      curr = curr->next;
    }
  }

  ImGui::PopStyleColor();
}

void ExchangeApp::render_asks() {
  const auto &order_book = engine_.get_order_book();
  const auto &asks = order_book.get_asks();

  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f)); // Red

  // Asks are sorted ascending (lowest price first) - we want to show lowest
  // first
  int row_count = 0;
  for (const auto &[price, level] : asks) {
    if (row_count++ > 20)
      break;
    ImGui::Text("Px: %llu | Vol: %llu", price, level.total_volume);

    Order *curr = level.head;
    while (curr) {
      ImGui::BulletText("id:%llu (qty:%llu)", curr->id, curr->quantity);
      curr = curr->next;
    }
  }

  ImGui::PopStyleColor();
}

} // namespace exchange
