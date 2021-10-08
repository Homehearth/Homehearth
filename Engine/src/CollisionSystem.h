#pragma once
class CollisionSystem
{
private:
    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator () (std::pair<T1, T2> const& pair) const
        {
            //std::size_t h1 = std::hash<T1>()(pair->first);
            //std::size_t h2 = std::hash<T2>()(pair->second);

            //return h1 ^ h2;
        }
    };
	std::unordered_set<std::pair<Entity, Entity>, pair_hash> m_CollisionPairs;
public:
	const std::unordered_set<std::pair<Entity, Entity>, pair_hash>& getCollisions() const;
	void addPair(const Entity e1, const Entity e2);

};

