#include "dense_index.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace dense_index;

// Modern approach: Define strong types explicitly
class EmployeeId {
    std::size_t id_;
public:
    explicit EmployeeId(std::size_t id = 0) : id_(id) {}
    std::size_t get() const { return id_; }
    auto operator<=>(const EmployeeId&) const = default;
};

class DepartmentId {
    std::size_t id_;
public:
    explicit DepartmentId(std::size_t id = 0) : id_(id) {}
    std::size_t get() const { return id_; }
    auto operator<=>(const DepartmentId&) const = default;
};

class ProjectId {
    std::size_t id_;
public:
    explicit ProjectId(std::size_t id = 0) : id_(id) {}
    std::size_t get() const { return id_; }
    auto operator<=>(const ProjectId&) const = default;
    ProjectId& operator++() { ++id_; return *this; }
};

// TeamMemberIndex is for indexing within a team (0 = first member, 1 = second, etc)
class TeamMemberIndex {
    std::size_t pos_;
public:
    explicit TeamMemberIndex(std::size_t pos = 0) : pos_(pos) {}
    std::size_t get() const { return pos_; }
    auto operator<=>(const TeamMemberIndex&) const = default;
    TeamMemberIndex& operator++() { ++pos_; return *this; }
};

namespace modern_example {

struct Employee {
    std::string name;
    DepartmentId department;
    double salary;
};

struct Department {
    std::string name;
    EmployeeId manager;
};

struct Project {
    std::string name;
    // This is a vector of EmployeeIds, indexed by TeamMemberIndex
    // So team_members[TeamMemberIndex(0)] gives you the first team member's EmployeeId
    DenseIndexedContainer<std::vector<EmployeeId>, TeamMemberIndex> team_members;
};

void demonstrate() {
    std::cout << "=== Modern Employee Management (Custom Strong Types) ===" << std::endl;

    // Create strongly-typed containers using the strong types directly
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
    Project web_project{"Website Redesign", {}};
    web_project.team_members.push_back(alice);    // Returns TeamMemberIndex(0)
    web_project.team_members.push_back(bob);      // Returns TeamMemberIndex(1)

    Project sales_project{"Sales Campaign", {}};
    sales_project.team_members.push_back(charlie); // Returns TeamMemberIndex(0)
    sales_project.team_members.push_back(diana);   // Returns TeamMemberIndex(1)

    projects.push_back(std::move(web_project));
    projects.push_back(std::move(sales_project));

    // Type safety examples - these would not compile:
    // employees[eng_dept];          // Error: DepartmentId != EmployeeId
    // departments[alice];           // Error: EmployeeId != DepartmentId
    // projects[TeamMemberIndex(0)]; // Error: TeamMemberIndex != ProjectId

    // Display information
    std::cout << "\nProjects and Teams:" << std::endl;
    for (ProjectId proj_id(0); proj_id.get() < projects.size(); ++proj_id) {
        const auto& project = projects[proj_id];
        std::cout << "  " << project.name << ":" << std::endl;

        // Iterate through team members by position
        for (TeamMemberIndex pos(0); pos.get() < project.team_members.size(); ++pos) {
            EmployeeId emp_id = project.team_members[pos];
            std::cout << "    Position " << pos.get() << ": "
                      << employees[emp_id].name << std::endl;
        }
    }

    // Or using range-based for (iterates values, not indices)
    std::cout << "\nTeam Lists (range-based iteration):" << std::endl;
    for (ProjectId proj_id(0); proj_id.get() < projects.size(); ++proj_id) {
        const auto& project = projects[proj_id];
        std::cout << "  " << project.name << ": ";
        for (const auto& emp_id : project.team_members) {
            std::cout << employees[emp_id].name << " ";
        }
        std::cout << std::endl;
    }

    // Example: Get the lead developer (first team member) of each project
    std::cout << "\nProject Leads:" << std::endl;
    for (ProjectId proj_id(0); proj_id.get() < projects.size(); ++proj_id) {
        const auto& project = projects[proj_id];
        if (!project.team_members.empty()) {
            // Access by specific position using TeamMemberIndex
            EmployeeId lead = project.team_members[TeamMemberIndex(0)];
            std::cout << "  " << project.name << " lead: "
                      << employees[lead].name << std::endl;
        }
    }
}

} // namespace modern_example

// For comparison: Tag-based approach (backward compatible but less explicit)
namespace tag_based_example {

struct EmployeeTag {};
struct DepartmentTag {};
struct ProjectTag {};
struct TeamMemberTag {};

using EmployeeId = StrongIndex<EmployeeTag>;
using DepartmentId = StrongIndex<DepartmentTag>;
using ProjectId = StrongIndex<ProjectTag>;
using TeamMemberIndex = StrongIndex<TeamMemberTag>;

struct Employee {
    std::string name;
    DepartmentId department;
};

void demonstrate() {
    std::cout << "\n=== Tag-Based Approach (Backward Compatible) ===" << std::endl;

    // Can use tags directly - they create StrongIndex types internally
    DenseIndexedContainer<std::vector<Employee>, EmployeeTag> employees;
    DenseIndexedContainer<std::vector<std::string>, DepartmentTag> departments;

    auto dept = departments.push_back("Engineering");
    auto emp = employees.emplace_back("Alice", dept);

    std::cout << "  Employee: " << employees[emp].name
              << " in " << departments[dept] << std::endl;

    std::cout << "  Note: Using tags creates StrongIndex<Tag> internally" << std::endl;
}

} // namespace tag_based_example

int main() {
    std::cout << "Dense Indexed Container - Modern API Examples\n" << std::endl;

    modern_example::demonstrate();
    tag_based_example::demonstrate();

    std::cout << "\n✅ Examples completed successfully!" << std::endl;

    return 0;
}