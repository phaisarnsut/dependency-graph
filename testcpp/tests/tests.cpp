#include "gtest/gtest.h"
#include "ecs/Component.h"
#include "ecs/EntityManager.h"

using namespace ecs;

struct Position : public Component<Position>
{
    Position(float X = 0.0, float Y = 0.0) : x(X), y(Y)
    {

    }

    float x;
    float y;
};

struct Velocity : public Component<Velocity>
{
    Velocity(float X = 0.0, float Y = 0.0) : x(X), y(Y)
    {

    }

    float x;
    float y;
};

struct Mass : public Component<Mass>
{
    Mass(float Value = 0.0) : value(Value)
    {

    }

    float value;
};

float getX(std::size_t i)
{
    return static_cast<float>(i);
}

float getY(std::size_t i)
{
    return static_cast<float>(i + 1);
}

float getVx(std::size_t i)
{
    return 2.0f * static_cast<float>(i);
}

float getVy(std::size_t i)
{
    return 2.0f * static_cast<float>(i + 1);
}

float getMass(std::size_t i)
{
    return 3.0f * static_cast<float>(i);
}

template<typename ...Ts>
std::vector<Entity> getEntitiesInEntitySet(const EntitySet<Ts...>& entitySet)
{
    auto entities = std::vector<Entity>();
    for (auto [entity, components] : entitySet)
        entities.push_back(entity);
    return entities;
}

class EntityManagerTest : public ::testing::TestWithParam<std::tuple<bool, std::size_t>>
{
protected:
    EntityManager manager;
};

TEST_P(EntityManagerTest, AddComponents)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        ASSERT_FALSE(manager.hasComponent<Position>(entity));
        ASSERT_FALSE(manager.hasComponents<Position>(entity));
        ASSERT_FALSE(std::as_const(manager).hasComponent<Position>(entity));
        ASSERT_FALSE(std::as_const(manager).hasComponents<Position>(entity));
        manager.addComponent<Position>(entity, getX(i), getY(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        ASSERT_TRUE(manager.hasEntity(entity));
        ASSERT_TRUE(manager.hasComponent<Position>(entity));
        ASSERT_FALSE(manager.hasComponent<Velocity>(entity));
        ASSERT_FALSE(manager.hasComponent<Mass>(entity));
        ASSERT_TRUE(manager.hasComponents<Position>(entity));
        ASSERT_FALSE(manager.hasComponents<Velocity>(entity));
        ASSERT_FALSE(manager.hasComponents<Mass>(entity));
        auto hasPositionAndVelocity = manager.hasComponents<Position, Velocity>(entity);
        ASSERT_FALSE(hasPositionAndVelocity);
        auto hasVelocityAndMass = manager.hasComponents<Velocity, Mass>(entity);
        ASSERT_FALSE(hasVelocityAndMass);
        const auto& position = manager.getComponent<Position>(entity);
        ASSERT_EQ(position.x, getX(i));
        ASSERT_EQ(position.y, getY(i));
        auto [anotherPosition] = manager.getComponents<Position>(entity);
        ASSERT_EQ(anotherPosition.x, getX(i));
        ASSERT_EQ(anotherPosition.y, getY(i));
    }
    // Const version
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        ASSERT_TRUE(std::as_const(manager).hasComponent<Position>(entity));
        ASSERT_FALSE(std::as_const(manager).hasComponent<Velocity>(entity));
        ASSERT_FALSE(std::as_const(manager).hasComponent<Mass>(entity));
        ASSERT_TRUE(std::as_const(manager).hasComponents<Position>(entity));
        ASSERT_FALSE(std::as_const(manager).hasComponents<Velocity>(entity));
        ASSERT_FALSE(std::as_const(manager).hasComponents<Mass>(entity));
        auto hasPositionAndVelocity = std::as_const(manager).hasComponents<Position, Velocity>(entity);
        ASSERT_FALSE(hasPositionAndVelocity);
        auto hasVelocityAndMass = std::as_const(manager).hasComponents<Velocity, Mass>(entity);
        ASSERT_FALSE(hasVelocityAndMass);
        const auto& position = std::as_const(manager).getComponent<Position>(entity);
        ASSERT_EQ(position.x, getX(i));
        ASSERT_EQ(position.y, getY(i));
        auto [anotherPosition] = std::as_const(manager).getComponents<Position>(entity);
        ASSERT_EQ(anotherPosition.x, getX(i));
        ASSERT_EQ(anotherPosition.y, getY(i));
    }
    auto entitySetSize = manager.getEntitySet<Position>().getSize();
    ASSERT_EQ(entitySetSize, nbEntities);
}

TEST_P(EntityManagerTest, AddAndModifyComponents)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        manager.addComponent<Position>(entity, getX(i), getY(i));
    }
    for (const auto& entity : entities)
    {
        auto& position = manager.getComponent<Position>(entity);
        position.x *= 2.0f;
        position.y *= 2.0f;
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        const auto& position = manager.getComponent<Position>(entity);
        ASSERT_EQ(position.x, 2.0f * getX(i));
        ASSERT_EQ(position.y, 2.0f * getY(i));
    }
    auto entitySetSize = manager.getEntitySet<Position>().getSize();
    ASSERT_EQ(entitySetSize, nbEntities);
}

TEST_P(EntityManagerTest, AddAndRemoveComponents)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        manager.addComponent<Position>(entity, getX(i), getY(i));
    }
    for (const auto& entity : entities)
        manager.removeComponent<Position>(entity);
    for (const auto& entity : entities)
    {
        ASSERT_FALSE(manager.hasComponent<Position>(entity));
        ASSERT_FALSE(manager.hasComponents<Position>(entity));
    }
    auto entitySetSize = manager.getEntitySet<Position>().getSize();
    ASSERT_EQ(entitySetSize, 0);
}

TEST_P(EntityManagerTest, AddAndRemoveSomeComponents)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        manager.addComponent<Position>(entity, getX(i), getY(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); i += 2)
        manager.removeComponent<Position>(entities[i]);
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 2 == 0)
        {
            ASSERT_FALSE(manager.hasComponent<Position>(entity));
            ASSERT_FALSE(manager.hasComponents<Position>(entity));
        }
        else
        {
            ASSERT_TRUE(manager.hasComponent<Position>(entity));
            ASSERT_TRUE(manager.hasComponents<Position>(entity));
            const auto& position = manager.getComponent<Position>(entity);
            ASSERT_EQ(position.x, getX(i));
            ASSERT_EQ(position.y, getY(i));
        }
    }
    auto entitySetSize = manager.getEntitySet<Position>().getSize();
    ASSERT_EQ(entitySetSize, nbEntities / 2);
}

TEST_P(EntityManagerTest, AddSeveralComponents)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        manager.addComponent<Position>(entity, getX(i), getY(i));
        manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        ASSERT_TRUE(manager.hasComponent<Position>(entity));
        ASSERT_TRUE(manager.hasComponent<Velocity>(entity));
        ASSERT_TRUE(manager.hasComponent<Mass>(entity));
        ASSERT_TRUE(manager.hasComponents<Position>(entity));
        ASSERT_TRUE(manager.hasComponents<Velocity>(entity));
        ASSERT_TRUE(manager.hasComponents<Mass>(entity));
        auto hasPositionAndVelocity = manager.hasComponents<Position, Velocity>(entity);
        ASSERT_TRUE(hasPositionAndVelocity);
        auto hasVelocityAndMass = manager.hasComponents<Velocity, Mass>(entity);
        ASSERT_TRUE(hasVelocityAndMass);
        auto hasAll = manager.hasComponents<Position, Velocity, Mass>(entity);
        ASSERT_TRUE(hasAll);
        auto [position, velocity, mass] = manager.getComponents<Position, Velocity, Mass>(entity);
        ASSERT_EQ(position.x, getX(i));
        ASSERT_EQ(position.y, getY(i));
        ASSERT_EQ(velocity.x, getVx(i));
        ASSERT_EQ(velocity.y, getVy(i));
        ASSERT_EQ(mass.value, getMass(i));
    }
    // Const version
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        ASSERT_TRUE(std::as_const(manager).hasComponent<Position>(entity));
        ASSERT_TRUE(std::as_const(manager).hasComponent<Velocity>(entity));
        ASSERT_TRUE(std::as_const(manager).hasComponent<Mass>(entity));
        ASSERT_TRUE(std::as_const(manager).hasComponents<Position>(entity));
        ASSERT_TRUE(std::as_const(manager).hasComponents<Velocity>(entity));
        ASSERT_TRUE(std::as_const(manager).hasComponents<Mass>(entity));
        auto hasPositionAndVelocity = std::as_const(manager).hasComponents<Position, Velocity>(entity);
        ASSERT_TRUE(hasPositionAndVelocity);
        auto hasVelocityAndMass = std::as_const(manager).hasComponents<Velocity, Mass>(entity);
        ASSERT_TRUE(hasVelocityAndMass);
        auto hasAll = std::as_const(manager).hasComponents<Position, Velocity, Mass>(entity);
        ASSERT_TRUE(hasAll);
        auto [position, velocity, mass] = std::as_const(manager).getComponents<Position, Velocity, Mass>(entity);
        ASSERT_EQ(position.x, getX(i));
        ASSERT_EQ(position.y, getY(i));
        ASSERT_EQ(velocity.x, getVx(i));
        ASSERT_EQ(velocity.y, getVy(i));
        ASSERT_EQ(mass.value, getMass(i));
    }
    auto entitySetSize = manager.getEntitySet<Position, Velocity, Mass>().getSize();
    ASSERT_EQ(entitySetSize, nbEntities);
}

TEST_P(EntityManagerTest, AddHeterogeneousEntities)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        if (i % 2 == 0)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 3 == 0)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 5 == 0)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 2 == 0)
        {
            ASSERT_TRUE(manager.hasComponent<Position>(entity));
            const auto& position = manager.getComponent<Position>(entity);
            ASSERT_EQ(position.x, getX(i));
            ASSERT_EQ(position.y, getY(i));
        }
        else
            ASSERT_FALSE(manager.hasComponent<Position>(entity));
        if (i % 3 == 0)
        {
            ASSERT_TRUE(manager.hasComponent<Velocity>(entity));
            const auto& velocity = manager.getComponent<Velocity>(entity);
            ASSERT_EQ(velocity.x, getVx(i));
            ASSERT_EQ(velocity.y, getVy(i));
        }
        else
            ASSERT_FALSE(manager.hasComponent<Velocity>(entity));
        if (i % 5 == 0)
        {
            ASSERT_TRUE(manager.hasComponents<Mass>(entity));
            const auto& mass = manager.getComponent<Mass>(entity);
            ASSERT_EQ(mass.value, getMass(i));
        }
        else
            ASSERT_FALSE(manager.hasComponents<Mass>(entity));
        auto hasPositionAndVelocity = manager.hasComponents<Position, Velocity>(entity);
        if (i % 6 == 0)
            ASSERT_TRUE(hasPositionAndVelocity);
        else
            ASSERT_FALSE(hasPositionAndVelocity);
        auto hasVelocityAndMass = manager.hasComponents<Velocity, Mass>(entity);
        if (i % 15 == 0)
            ASSERT_TRUE(hasVelocityAndMass);
        else
            ASSERT_FALSE(hasVelocityAndMass);
        auto hasAll = manager.hasComponents<Position, Velocity, Mass>(entity);
        if (i % 30 == 0)
            ASSERT_TRUE(hasAll);
        else
            ASSERT_FALSE(hasAll);
    }
    auto entitySetSize = manager.getEntitySet<Position, Velocity, Mass>().getSize();
    ASSERT_EQ(entitySetSize, (nbEntities - 1) / 30 + 1);
}

TEST_P(EntityManagerTest, AddSeveralComponentsAndRemoveSome)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        manager.addComponent<Position>(entity, getX(i), getY(i));
        manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 4 >= 1)
            manager.removeComponent<Position>(entity);
        if (i % 4 >= 2)
            manager.removeComponent<Velocity>(entity);
        if (i % 4 >= 3)
            manager.removeComponent<Mass>(entity);
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 4 < 1)
        {
            ASSERT_TRUE(manager.hasComponent<Position>(entity));
            const auto& position = manager.getComponent<Position>(entity);
            ASSERT_EQ(position.x, getX(i));
            ASSERT_EQ(position.y, getY(i));
        }
        else if (i % 4 < 2)
        {
            ASSERT_TRUE(manager.hasComponent<Velocity>(entity));
            const auto& velocity = manager.getComponent<Velocity>(entity);
            ASSERT_EQ(velocity.x, getVx(i));
            ASSERT_EQ(velocity.y, getVy(i));
        }
        else if (i % 4 < 3)
        {
            ASSERT_TRUE(manager.hasComponent<Mass>(entity));
            const auto& mass = manager.getComponent<Mass>(entity);
            ASSERT_EQ(mass.value, getMass(i));
        }
    }
    auto entitySetSize = manager.getEntitySet<Position, Velocity, Mass>().getSize();
    ASSERT_EQ(entitySetSize, (nbEntities - 1) / 4 + 1);
}

TEST_P(EntityManagerTest, AddAndRemoveEntities)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        if (i % 4 >= 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 >= 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 >= 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        manager.removeEntity(entity);
        ASSERT_FALSE(manager.hasEntity(entity));
    }
    auto entitySetSize = manager.getEntitySet<Position, Velocity, Mass>().getSize();
    ASSERT_EQ(entitySetSize, 0);
}

TEST_P(EntityManagerTest, AddAndRemoveSomeEntities)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        if (i % 4 >= 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 >= 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 >= 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 3 == 0)
            manager.removeEntity(entity);
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 3 != 0)
        {
            ASSERT_TRUE(manager.hasEntity(entity));
            if (i % 4 >= 1)
            {
                ASSERT_TRUE(manager.hasComponent<Position>(entity));
                const auto& position = manager.getComponent<Position>(entity);
                ASSERT_EQ(position.x, getX(i));
                ASSERT_EQ(position.y, getY(i));
            }
            else if (i % 4 >= 2)
            {
                ASSERT_TRUE(manager.hasComponent<Velocity>(entity));
                const auto& velocity = manager.getComponent<Velocity>(entity);
                ASSERT_EQ(velocity.x, getVx(i));
                ASSERT_EQ(velocity.y, getVy(i));
            }
            else if (i % 4 >= 3)
            {
                ASSERT_TRUE(manager.hasComponent<Mass>(entity));
                const auto& mass = manager.getComponent<Mass>(entity);
                ASSERT_EQ(mass.value, getMass(i));
            }
        }
        else
            ASSERT_FALSE(manager.hasEntity(entity));
    }
    auto entitySetSize = manager.getEntitySet<Position, Velocity, Mass>().getSize();
    if (nbEntities > 3)
        ASSERT_EQ(entitySetSize, (nbEntities - 4) / 4 - (nbEntities - 4) / 12);
    else
        ASSERT_EQ(entitySetSize, 0);
}

TEST_P(EntityManagerTest, AddRemoveAndAddEntities)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        if (i % 4 >= 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 >= 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 >= 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 3 == 0)
            manager.removeEntity(entity);
    }
    auto otherEntities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = otherEntities.emplace_back(manager.createEntity());
        if (i % 4 < 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 < 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 < 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    for (auto i = std::size_t(0); i < entities.size(); ++i)
    {
        auto entity = entities[i];
        if (i % 3 != 0)
        {
            ASSERT_TRUE(manager.hasEntity(entity));
            if (i % 4 >= 1)
            {
                ASSERT_TRUE(manager.hasComponent<Position>(entity));
                const auto& position = manager.getComponent<Position>(entity);
                ASSERT_EQ(position.x, getX(i));
                ASSERT_EQ(position.y, getY(i));
            }
            if (i % 4 >= 2)
            {
                ASSERT_TRUE(manager.hasComponent<Velocity>(entity));
                const auto& velocity = manager.getComponent<Velocity>(entity);
                ASSERT_EQ(velocity.x, getVx(i));
                ASSERT_EQ(velocity.y, getVy(i));
            }
            if (i % 4 >= 3)
            {
                ASSERT_TRUE(manager.hasComponent<Mass>(entity));
                const auto& mass = manager.getComponent<Mass>(entity);
                ASSERT_EQ(mass.value, getMass(i));
            }
        }
    }
    for (auto i = std::size_t(0); i < otherEntities.size(); ++i)
    {
        auto entity = otherEntities[i];
        ASSERT_TRUE(manager.hasEntity(entity));
        if (i % 4 < 1)
        {
            ASSERT_TRUE(manager.hasComponent<Position>(entity));
            const auto& position = manager.getComponent<Position>(entity);
            ASSERT_EQ(position.x, getX(i));
            ASSERT_EQ(position.y, getY(i));
        }
        if (i % 4 < 2)
        {
            ASSERT_TRUE(manager.hasComponent<Velocity>(entity));
            const auto& velocity = manager.getComponent<Velocity>(entity);
            ASSERT_EQ(velocity.x, getVx(i));
            ASSERT_EQ(velocity.y, getVy(i));
        }
        if (i % 4 < 3)
        {
            ASSERT_TRUE(manager.hasComponent<Mass>(entity));
            const auto& mass = manager.getComponent<Mass>(entity);
            ASSERT_EQ(mass.value, getMass(i));
        }
    }
    auto entitySetSize = manager.getEntitySet<Position, Velocity, Mass>().getSize();
    if (nbEntities > 3)
        ASSERT_EQ(entitySetSize, (nbEntities - 4) / 4 - (nbEntities - 4) / 12 + (nbEntities - 1) / 4 + 1);
    else
        ASSERT_EQ(entitySetSize, (nbEntities - 1) / 4 + 1);
}

TEST_P(EntityManagerTest, EntitySet)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entitiesWithOne = std::vector<Entity>();
    auto entitiesWithTwo = std::vector<Entity>();
    auto entitiesWithThree = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = manager.createEntity();
        if (i % 4 >= 1)
        {
            manager.addComponent<Position>(entity, getX(i), getY(i));
            entitiesWithOne.push_back(entity);
        }
        if (i % 4 >= 2)
        {
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
            entitiesWithTwo.push_back(entity);
        }
        if (i % 4 >= 3)
        {
            manager.addComponent<Mass>(entity, getMass(i));
            entitiesWithThree.push_back(entity);
        }
    }
    // Tests
    auto entitiesInEntitySet = std::vector<Entity>();
    // One
    entitiesInEntitySet = getEntitiesInEntitySet(manager.getEntitySet<Position>());
    ASSERT_EQ(entitiesWithOne.size(), entitiesInEntitySet.size());
    std::sort(std::begin(entitiesWithOne), std::end(entitiesWithOne));
    std::sort(std::begin(entitiesInEntitySet), std::end(entitiesInEntitySet));
    ASSERT_EQ(entitiesWithOne, entitiesInEntitySet);
    // Two
    entitiesInEntitySet = getEntitiesInEntitySet(manager.getEntitySet<Position, Velocity>());
    ASSERT_EQ(entitiesWithTwo.size(), entitiesInEntitySet.size());
    std::sort(std::begin(entitiesWithTwo), std::end(entitiesWithTwo));
    std::sort(std::begin(entitiesInEntitySet), std::end(entitiesInEntitySet));
    ASSERT_EQ(entitiesWithTwo, entitiesInEntitySet);
    // Three
    entitiesInEntitySet = getEntitiesInEntitySet(manager.getEntitySet<Position, Velocity, Mass>());
    ASSERT_EQ(entitiesWithThree.size(), entitiesInEntitySet.size());
    std::sort(std::begin(entitiesWithThree), std::end(entitiesWithThree));
    std::sort(std::begin(entitiesInEntitySet), std::end(entitiesInEntitySet));
    ASSERT_EQ(entitiesWithThree, entitiesInEntitySet);
}

TEST_P(EntityManagerTest, EntityAddedListener)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entitiesWithOne = std::vector<Entity>();
    auto entitiesWithTwo = std::vector<Entity>();
    auto entitiesWithThree = std::vector<Entity>();
    // Add listeners
    manager.getEntitySet<Position>().addEntityAddedListener([&entitiesWithOne](auto entity)
    {
        entitiesWithOne.push_back(entity);
    });
    manager.getEntitySet<Position, Velocity>().addEntityAddedListener([&entitiesWithTwo](auto entity)
    {
        entitiesWithTwo.push_back(entity);
    });
    manager.getEntitySet<Position, Velocity, Mass>().addEntityAddedListener([&entitiesWithThree](auto entity)
    {
        entitiesWithThree.push_back(entity);
    });
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = manager.createEntity();
        if (i % 4 >= 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 >= 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 >= 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    // Tests
    auto entitiesInEntitySet = std::vector<Entity>();
    // One
    entitiesInEntitySet = getEntitiesInEntitySet(manager.getEntitySet<Position>());
    ASSERT_EQ(entitiesWithOne.size(), entitiesInEntitySet.size());
    std::sort(std::begin(entitiesWithOne), std::end(entitiesWithOne));
    std::sort(std::begin(entitiesInEntitySet), std::end(entitiesInEntitySet));
    ASSERT_EQ(entitiesWithOne, entitiesInEntitySet);
    // Two
    entitiesInEntitySet = getEntitiesInEntitySet(manager.getEntitySet<Position, Velocity>());
    ASSERT_EQ(entitiesWithTwo.size(), entitiesInEntitySet.size());
    std::sort(std::begin(entitiesWithTwo), std::end(entitiesWithTwo));
    std::sort(std::begin(entitiesInEntitySet), std::end(entitiesInEntitySet));
    ASSERT_EQ(entitiesWithTwo, entitiesInEntitySet);
    // Three
    entitiesInEntitySet = getEntitiesInEntitySet(manager.getEntitySet<Position, Velocity, Mass>());
    ASSERT_EQ(entitiesWithThree.size(), entitiesInEntitySet.size());
    std::sort(std::begin(entitiesWithThree), std::end(entitiesWithThree));
    std::sort(std::begin(entitiesInEntitySet), std::end(entitiesInEntitySet));
    ASSERT_EQ(entitiesWithThree, entitiesInEntitySet);
}

TEST_P(EntityManagerTest, EntityRemovedListener)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto counterPosition = std::size_t(0);
    auto counterVelocity = std::size_t(0);
    auto counterMass = std::size_t(0);
    // Add listeners
    manager.getEntitySet<Position>().addEntityAddedListener([&counterPosition]([[maybe_unused]] auto entity)
    {
        ++counterPosition;
    });
    manager.getEntitySet<Position, Velocity>().addEntityAddedListener([&counterVelocity]([[maybe_unused]] auto entity)
    {
        ++counterVelocity;
    });
    manager.getEntitySet<Position, Velocity, Mass>().addEntityAddedListener([&counterMass]([[maybe_unused]] auto entity)
    {
        ++counterMass;
    });
    manager.getEntitySet<Position>().addEntityRemovedListener([&counterPosition]([[maybe_unused]] auto entity)
    {
        --counterPosition;
    });
    manager.getEntitySet<Position, Velocity>().addEntityRemovedListener([&counterVelocity]([[maybe_unused]] auto entity)
    {
        --counterVelocity;
    });
    manager.getEntitySet<Position, Velocity, Mass>().addEntityRemovedListener([&counterMass]([[maybe_unused]] auto entity)
    {
        --counterMass;
    });
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = manager.createEntity();
        if (i % 4 >= 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 >= 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 >= 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    // Tests
    ASSERT_EQ(manager.getEntitySet<Position>().getSize(), counterPosition);
    ASSERT_EQ(manager.getEntitySet<Velocity>().getSize(), counterVelocity);
    ASSERT_EQ(manager.getEntitySet<Mass>().getSize(), counterMass);
}

TEST_P(EntityManagerTest, Visitor)
{
    auto [reserve, nbEntities] = GetParam();
    if (reserve)
        manager.reserve(nbEntities);
    auto entities = std::vector<Entity>();
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = entities.emplace_back(manager.createEntity());
        if (i % 4 >= 1)
            manager.addComponent<Position>(entity, getX(i), getY(i));
        if (i % 4 >= 2)
            manager.addComponent<Velocity>(entity, getVx(i), getVy(i));
        if (i % 4 >= 3)
            manager.addComponent<Mass>(entity, getMass(i));
    }
    // Visit
    auto counterPosition = std::size_t(0);
    auto counterVelocity = std::size_t(0);
    auto counterMass = std::size_t(0);
    auto visitor = Visitor();
    visitor.setHandler<Position>([&counterPosition]([[maybe_unused]] const auto& position)
    {
        ++counterPosition;
    });
    visitor.setHandler<Velocity>([&counterVelocity]([[maybe_unused]] const auto& velocity)
    {
        ++counterVelocity;
    });
    visitor.setHandler<Mass>([&counterMass]([[maybe_unused]] const auto& mass)
    {
        ++counterMass;
    });
    for (auto entity : entities)
        manager.visitEntity(entity, visitor);
    // Tests
    ASSERT_EQ(manager.getEntitySet<Position>().getSize(), counterPosition);
    ASSERT_EQ(manager.getEntitySet<Velocity>().getSize(), counterVelocity);
    ASSERT_EQ(manager.getEntitySet<Mass>().getSize(), counterMass);
}

// Seems that I use an old version of googletest, should be replaced by INSTANTIATE_TEST_SUITE in latter version
INSTANTIATE_TEST_CASE_P(ReserveAndNbEntities, EntityManagerTest, ::testing::Combine(::testing::Values(false, true), ::testing::Values(1, 100, 10000)));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}