#include <SFML/Graphics.hpp>
#include <deque>
#include <random>
#include <ranges>
#include <algorithm>
#include <iostream>


constexpr int gridWidth = 80;
constexpr int gridHeight = 60;
constexpr int cellSize = 10;
constexpr int windowWidth = gridWidth * cellSize;
constexpr int windowHeight = gridHeight * cellSize;

class GameObject {
public:
    virtual void Update() = 0;
    virtual void Render(sf::RenderWindow& window) = 0;
    virtual ~GameObject() = default;
};

class Food final : public GameObject {
public:
    Food()
        : rng(std::random_device{}()),
        distX(0, gridWidth - 1),
        distY(0, gridHeight - 1)
    {
        Respawn({});
    }

    void Update() override {
    }

    void Render(sf::RenderWindow& window) override {
        sf::RectangleShape cell{ sf::Vector2f(cellSize - 1.f, cellSize - 1.f) };
        cell.setFillColor(sf::Color::Green);
        cell.setPosition(sf::Vector2f(
            static_cast<float>(position.x * cellSize),
            static_cast<float>(position.y * cellSize)
        ));
        window.draw(cell);
    }

    [[nodiscard]] sf::Vector2i GetPosition() const {
        return position;
    }

    void Respawn(const std::deque<sf::Vector2i>& snakeSegments) {
        do {
            position = { distX(rng), distY(rng) };
        } while (std::find(snakeSegments.begin(), snakeSegments.end(), position) != snakeSegments.end());
    }

private:
    sf::Vector2i position;
    std::mt19937 rng;
    std::uniform_int_distribution<> distX;
    std::uniform_int_distribution<> distY;
};


enum class Direction { Up, Down, Left, Right };

class Snake final : public GameObject {
public:
    explicit Snake(Food& f) : food(f) {
        segments.emplace_back(gridWidth / 2, gridHeight / 2);
    }

    void Update() override {
        HandleInput();
        HandleHeadMove();
        UpdateAlive();
        HandleFood();
    }

    void Render(sf::RenderWindow& window) override {
        sf::RectangleShape cell{ sf::Vector2f(cellSize - 1.f, cellSize - 1.f) };
        cell.setFillColor(sf::Color::Magenta);
        for (const auto& segment : segments) {
            cell.setPosition(sf::Vector2f(
                static_cast<float>(segment.x * cellSize),
                static_cast<float>(segment.y * cellSize)
            ));
            window.draw(cell);
        }
    }

    [[nodiscard]] bool IsAlive() const { return alive; }

private:
    std::deque<sf::Vector2i> segments{3};
    Direction dir = Direction::Right;
    Food& food;
    bool alive = true;
    bool move = true;

    void HandleInput() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && dir != Direction::Down) {
            dir = Direction::Up;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && dir != Direction::Up) {
            dir = Direction::Down;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && dir != Direction::Right) {
            dir = Direction::Left;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && dir != Direction::Left) {
            dir = Direction::Right;
        }
    }

    void HandleHeadMove() {
        auto head = segments.front();
        int x = head.x;
        int y = head.y;

        switch (dir) {
        case Direction::Up:    --y; break;
        case Direction::Down:  ++y; break;
        case Direction::Left:  --x; break;
        case Direction::Right: ++x; break;
        }
        head = { x, y };
        segments.push_front(head);
    }

    void UpdateAlive() {
        const auto head = segments.front();
        const auto isHorizOut = head.x < 0 || head.x >= gridWidth;
        const auto isVertOut = head.y < 0 || head.y >= gridHeight;
        const auto isEatingItself = std::find(std::next(segments.begin()), segments.end(), head) != segments.end();
        alive = !isHorizOut && !isVertOut && !isEatingItself;
    }

    void HandleFood()
    {
        const auto head = segments.front();
        if (head == food.GetPosition()) {
            food.Respawn(segments);
        }
        else {
            segments.pop_back();
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode((windowWidth), (windowHeight)), "Wurm the Game");
    window.setFramerateLimit(30);

    sf::Clock clock;
    float timer = 0;
    constexpr float delay = 0.1f;
    
    bool isPaused = false;
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Nie mozna zaladowac czcionki!" << std::endl;
        return -1;
    }

    sf::Text pauseText("PAUZA", font, 48);
    pauseText.setFillColor(sf::Color::Green);
    pauseText.setPosition(300, 250);

    sf::Text gameOver("KONIEC GRY", font, 48);
    pauseText.setFillColor(sf::Color::Red);
    pauseText.setPosition(300, 250);

    Food food;
    Snake snake(food);

    while (window.isOpen()) {
        sf::Event event;
        while (auto eventOpt = window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                isPaused = !isPaused;
            }
        }

        if (!isPaused) {
            timer += clock.restart().asSeconds();
            if (timer < delay) continue;
            timer = 0.f;

            if (snake.IsAlive()) { window.draw(gameOver); }
            if (!snake.IsAlive()) { continue; }

            // update game objects
            snake.Update();
            food.Update();
        }

        // render all
        window.clear(sf::Color::Black);
        snake.Render(window);
        food.Render(window);

        if (isPaused)
        {
            window.draw(pauseText);
        }

        window.display();
    }

    return 0;
}