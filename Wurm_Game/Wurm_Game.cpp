#include <SFML/Graphics.hpp>
#include <deque>
#include <random>
#include <ranges>

constexpr int gridWidth = 40;
constexpr int gridHeight = 30;
constexpr int cellSize = 10;
constexpr int windowWidth = gridWidth * cellSize;
constexpr int windowHeight = gridHeight * cellSize;

int main() {
    sf::RenderWindow window( sf::VideoMode({ (windowWidth),(windowHeight) }), "Wurm the Game" );
    window.setFramerateLimit(30);

    sf::Clock clock;
    float timer = 0;
    constexpr float delay = 0.1f;

    Food food;
    Snake snake(food);

    while (window.isOpen()) {
        while (auto eventOpt = window.pollEvent()) { //błąd
            const auto& event = *eventOpt;
            if (event.is<sf::Event::Closed>()) { //błąd
                window.close();
            }
        }

        timer += clock.restart().asSeconds();
        if (timer < delay) continue;
        timer = 0.f;

        if (!snake.IsAlive()) continue;

        // update game objects
        snake.Update();
        food.Update();

        // render all
        window.clear(sf::Color::Black);
        snake.Render(window);
        food.Render(window);
        window.display();
    }

    return 0;
}

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
        cell.setFillColor(sf::Color::Red);
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
        } while (std::ranges::find(snakeSegments, position) != snakeSegments.end());
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
        cell.setFillColor(sf::Color::Green);
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
    std::deque<sf::Vector2i> segments;
    Direction dir = Direction::Right;
    Food& food;
    bool alive = true;

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