#include "Game.hpp"
#include "Color.hpp"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <nlohmann/json.hpp>

Game::Game()
  : m_window{ sf::VideoMode(1200, 1000), "Pursuit game" }
  , m_evader{ 10.f, Color::SeaColor }
  , m_pursuer{ 8.f, Color::CarrotOrange }
  , m_target{ sf::Vector2f{ 100.f, 50.f }, Color::SandColor }
  , m_config{ "config.json" }
{
  float target_x =
    m_window.getSize().x - static_cast<float>(m_window.getSize().x) / 2 - 50.f;
  float target_y =
    m_window.getSize().y - static_cast<float>(m_window.getSize().y) / 2 - 25.f;
  m_target.setPos(target_x, target_y);
  m_evader.setPos(target_x - 300.f, target_y + 25.f);
  m_pursuer.setPos(target_x + 100.f, target_y + 150.f);
}

sf::Vector2f Game::getNearestPath() const
{
  sf::Vector2f dest;
  for (int i = 0; i < m_target.getPointCount(); ++i) {
    sf::Vector2f point =
      sf::Vector2f{ m_target.getPoint(i).x + m_target.getPos().x,
                    m_target.getPoint(i).y + m_target.getPos().y };
    std::cout << point.y << '\n';
    if (dest.x == 0 && dest.y == 0) {
      dest = point;
      continue;
    }
    sf::Vector2f vectorTo = point - m_evader.getPos();
    sf::Vector2f destPoint = dest - m_evader.getPos();
    double vectorToLength = std::pow(vectorTo.x, 2) + std::pow(vectorTo.y, 2);
    double destLength = std::pow(destPoint.x, 2) + std::pow(destPoint.y, 2);
    dest = vectorToLength < destLength ? point : dest;
  }
  std::cout << dest.x << ' ' << dest.y << '\n';
  return dest;
}

float Game::getDistance(sf::Vector2f origin, sf::Vector2f end)
{
  sf::Vector2f distance = end - origin;
  return distance.x * distance.x + distance.y * distance.y;
}

void Game::run()
{
  auto move = [](float x, sf::Vector2f currentPosition, sf::Vector2f dest) {
    return (dest.y * (x - currentPosition.x) +
            currentPosition.y * (dest.x - x)) /
           (dest.x - currentPosition.x);
  };

  sf::Vector2f dest = getNearestPath();
  float new_x = (dest.x + m_evader.getPos().x) / 2;
  float new_y = move(new_x, m_evader.getPos(), dest);
  sf::Vector2f optimal_dest = sf::Vector2f{ new_x, new_y };
  m_window.loop([this, move, dest, optimal_dest]() {
    sf::Vector2f eCurrentPosition = m_evader.getPos();
    m_evader.setPos(
      eCurrentPosition.x + m_config.speed,
      move(eCurrentPosition.x + m_config.speed, eCurrentPosition, dest));

    sf::Vector2f pCurrentPosition = m_pursuer.getPos();
    float xcord = pCurrentPosition.x < eCurrentPosition.x
                    ? pCurrentPosition.x + m_config.speed
                    : pCurrentPosition.x - m_config.speed;

    if (pCurrentPosition.x < eCurrentPosition.x) {
      m_pursuer.setPos(xcord, move(xcord, pCurrentPosition, eCurrentPosition));
      m_window.drawTrajectory(
        m_pursuer, pCurrentPosition, eCurrentPosition, move);
    } else {
      m_pursuer.setPos(xcord, move(xcord, pCurrentPosition, optimal_dest));
      m_window.drawTrajectory(m_pursuer, pCurrentPosition, optimal_dest, move);
    }

    Actor dest_area{ 5.f, sf::Color::White };
    dest_area.setPos(optimal_dest.x, optimal_dest.y);
    m_window.draw(dest_area);
    m_window.draw(m_evader);
    m_window.drawTrajectory(m_evader, eCurrentPosition, dest, move);
    m_window.draw(m_pursuer);
    m_window.draw(m_target);
    m_window.display();
  });
}

Game::~Game() {}