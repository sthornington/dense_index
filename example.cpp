#include "dense_index.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <map>

using namespace dense_index;

// Example 1: Employee Management System
namespace employee_example {

struct EmployeeTag {};
struct DepartmentTag {};
struct ProjectTag {};

using EmployeeId = StrongIndex<EmployeeTag>;
using DepartmentId = StrongIndex<DepartmentTag>;
using ProjectId = StrongIndex<ProjectTag>;

struct Employee {
    std::string name;
    DepartmentId department;
    double salary;
};

struct Department {
    std::string name;
    EmployeeId manager;
};

// Strong type for team member position (0, 1, 2, etc in the team)
struct TeamPositionTag {};
using TeamPosition = StrongIndex<TeamPositionTag>;

struct Project {
    std::string name;
    // Vector of employee IDs, indexed by team position (not employee ID!)
    DenseIndexedContainer<std::vector<EmployeeId>, TeamPosition> team_members;
};

void demonstrate() {
    std::cout << "=== Employee Management System Example ===" << std::endl;

    // Create strongly-typed containers using strong index types
    DenseIndexedContainer<std::vector<Employee>, EmployeeId> employees;
    DenseIndexedContainer<std::vector<Department>, DepartmentId> departments;
    DenseIndexedContainer<std::vector<Project>, ProjectId> projects;

    // Add departments
    auto eng_dept = departments.emplace_back("Engineering", EmployeeId{});
    auto sales_dept = departments.emplace_back("Sales", EmployeeId{});

    // Add employees
    auto alice = employees.emplace_back("Alice Smith", eng_dept, 95000);
    auto bob = employees.emplace_back("Bob Jones", eng_dept, 85000);
    auto charlie = employees.emplace_back("Charlie Brown", sales_dept, 75000);
    auto diana = employees.emplace_back("Diana Prince", sales_dept, 90000);

    // Set department managers
    departments[eng_dept].manager = alice;
    departments[sales_dept].manager = diana;

    // Create projects with team members
    DenseIndexedContainer<std::vector<EmployeeId>, TeamPosition> web_team;
    web_team.push_back(alice);
    web_team.push_back(bob);

    DenseIndexedContainer<std::vector<EmployeeId>, TeamPosition> sales_team;
    sales_team.push_back(charlie);
    sales_team.push_back(diana);

    auto project1 = projects.emplace_back("Website Redesign", std::move(web_team));
    auto project2 = projects.emplace_back("Sales Campaign", std::move(sales_team));

    // Type safety in action - these would not compile:
    // employees[eng_dept];  // Error: wrong index type
    // departments[alice];   // Error: wrong index type
    // projects[0];         // Error: raw index not allowed

    // Display information
    std::cout << "\nDepartments and Managers:" << std::endl;
    for (DepartmentId dept_id{}; dept_id.value() < departments.size(); ++dept_id) {
        const auto& dept = departments[dept_id];
        const auto& manager = employees[dept.manager];
        std::cout << "  " << dept.name << " - Manager: " << manager.name << std::endl;
    }

    std::cout << "\nProjects and Teams:" << std::endl;
    for (ProjectId proj_id{}; proj_id.value() < projects.size(); ++proj_id) {
        const auto& project = projects[proj_id];
        std::cout << "  " << project.name << " - Team: ";
        // team_members is a DenseIndexedContainer, so we can iterate its values directly
        for (const auto& emp_id : project.team_members) {
            std::cout << employees[emp_id].name << " ";
        }
        std::cout << std::endl;
    }

    // Calculate department salaries
    std::cout << "\nDepartment Salary Totals:" << std::endl;
    std::map<DepartmentId, double> dept_salaries;
    for (EmployeeId emp_id{}; emp_id.value() < employees.size(); ++emp_id) {
        dept_salaries[employees[emp_id].department] += employees[emp_id].salary;
    }

    for (const auto& [dept_id, total] : dept_salaries) {
        std::cout << "  " << departments[dept_id].name << ": $" << total << std::endl;
    }
}

} // namespace employee_example

// Example 2: Game Entity System
namespace game_example {

struct EntityTag {};
struct ComponentTag {};
struct SystemTag {};

using EntityId = StrongIndex<EntityTag>;
using ComponentId = StrongIndex<ComponentTag>;
using SystemId = StrongIndex<SystemTag>;

struct Transform {
    float x, y, z;
    float rotation;
};

struct Health {
    int current;
    int max;
};

struct Entity {
    std::string name;
    ComponentId transform_component;
    ComponentId health_component;
    bool active;
};

void demonstrate() {
    std::cout << "\n=== Game Entity System Example ===" << std::endl;

    DenseIndexedContainer<std::vector<Entity>, EntityId> entities;
    DenseIndexedContainer<std::vector<Transform>, ComponentId> transforms;
    DenseIndexedContainer<std::vector<Health>, ComponentId> healths;

    // Create game entities with components
    auto player_transform = transforms.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
    auto player_health = healths.emplace_back(100, 100);
    auto player = entities.emplace_back("Player", player_transform, player_health, true);

    auto enemy_transform = transforms.emplace_back(10.0f, 0.0f, 5.0f, 180.0f);
    auto enemy_health = healths.emplace_back(50, 50);
    auto enemy = entities.emplace_back("Enemy", enemy_transform, enemy_health, true);

    // Update game state
    std::cout << "\nInitial State:" << std::endl;
    for (EntityId id{}; id.value() < entities.size(); ++id) {
        const auto& entity = entities[id];
        const auto& transform = transforms[entity.transform_component];
        const auto& health = healths[entity.health_component];

        std::cout << "  " << entity.name
                  << " - Pos(" << transform.x << "," << transform.y << "," << transform.z << ")"
                  << " - Health: " << health.current << "/" << health.max << std::endl;
    }

    // Simulate damage
    healths[entities[enemy].health_component].current -= 20;

    // Move player
    transforms[entities[player].transform_component].x += 5.0f;

    std::cout << "\nAfter Update:" << std::endl;
    for (EntityId id{}; id.value() < entities.size(); ++id) {
        const auto& entity = entities[id];
        const auto& transform = transforms[entity.transform_component];
        const auto& health = healths[entity.health_component];

        std::cout << "  " << entity.name
                  << " - Pos(" << transform.x << "," << transform.y << "," << transform.z << ")"
                  << " - Health: " << health.current << "/" << health.max << std::endl;
    }
}

} // namespace game_example

// Example 3: Graph Data Structure
namespace graph_example {

struct NodeTag {};
struct EdgeTag {};

using NodeId = StrongIndex<NodeTag>;
using EdgeId = StrongIndex<EdgeTag>;

struct Node {
    std::string label;
    int value;
};

struct Edge {
    NodeId from;
    NodeId to;
    double weight;
};

void demonstrate() {
    std::cout << "\n=== Graph Data Structure Example ===" << std::endl;

    DenseIndexedContainer<std::vector<Node>, NodeId> nodes;
    DenseIndexedContainer<std::vector<Edge>, EdgeId> edges;

    // Build a graph
    auto node_a = nodes.emplace_back("A", 10);
    auto node_b = nodes.emplace_back("B", 20);
    auto node_c = nodes.emplace_back("C", 30);
    auto node_d = nodes.emplace_back("D", 40);

    // Add edges
    edges.emplace_back(node_a, node_b, 1.5);
    edges.emplace_back(node_b, node_c, 2.0);
    edges.emplace_back(node_c, node_d, 1.0);
    edges.emplace_back(node_a, node_c, 3.5);
    edges.emplace_back(node_b, node_d, 2.5);

    // Find neighbors of each node
    std::cout << "\nGraph Structure:" << std::endl;
    for (NodeId node_id{}; node_id.value() < nodes.size(); ++node_id) {
        std::cout << "Node " << nodes[node_id].label << " connects to: ";

        for (EdgeId edge_id{}; edge_id.value() < edges.size(); ++edge_id) {
            const auto& edge = edges[edge_id];
            if (edge.from == node_id) {
                std::cout << nodes[edge.to].label << " (weight: " << edge.weight << ") ";
            }
        }
        std::cout << std::endl;
    }

    // Calculate total edge weight
    double total_weight = 0.0;
    for (const auto& edge : edges) {
        total_weight += edge.weight;
    }
    std::cout << "\nTotal graph weight: " << total_weight << std::endl;
}

} // namespace graph_example

// Example 4: Matrix Operations with Row/Column Indices
namespace matrix_example {

struct RowTag {};
struct ColTag {};

using RowIndex = StrongIndex<RowTag>;
using ColIndex = StrongIndex<ColTag>;

template<typename T>
class Matrix {
private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<T> data_;

public:
    Matrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols) {}

    T& at(RowIndex row, ColIndex col) {
        return data_[row.value() * cols_ + col.value()];
    }

    const T& at(RowIndex row, ColIndex col) const {
        return data_[row.value() * cols_ + col.value()];
    }

    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
};

void demonstrate() {
    std::cout << "\n=== Matrix with Strong Typing Example ===" << std::endl;

    Matrix<double> matrix(3, 4);

    // Initialize matrix
    for (RowIndex row{}; row.value() < matrix.rows(); ++row) {
        for (ColIndex col{}; col.value() < matrix.cols(); ++col) {
            matrix.at(row, col) = row.value() * 10 + col.value();
        }
    }

    // These would not compile:
    // matrix.at(ColIndex(0), RowIndex(0));  // Error: wrong order
    // matrix.at(0, 0);                      // Error: raw indices

    // Display matrix
    std::cout << "\nMatrix contents:" << std::endl;
    for (RowIndex row{}; row.value() < matrix.rows(); ++row) {
        for (ColIndex col{}; col.value() < matrix.cols(); ++col) {
            std::cout << matrix.at(row, col) << "\t";
        }
        std::cout << std::endl;
    }

    // Row sum
    std::cout << "\nRow sums:" << std::endl;
    for (RowIndex row{}; row.value() < matrix.rows(); ++row) {
        double sum = 0.0;
        for (ColIndex col{}; col.value() < matrix.cols(); ++col) {
            sum += matrix.at(row, col);
        }
        std::cout << "Row " << row.value() << ": " << sum << std::endl;
    }
}

} // namespace matrix_example

int main() {
    std::cout << "Dense Indexed Container - Practical Examples\n" << std::endl;
    std::cout << "This demonstrates how strong typing prevents index mix-ups" << std::endl;
    std::cout << "and makes code more self-documenting and maintainable.\n" << std::endl;

    employee_example::demonstrate();
    game_example::demonstrate();
    graph_example::demonstrate();
    matrix_example::demonstrate();

    std::cout << "\n✅ All examples completed successfully!" << std::endl;

    return 0;
}