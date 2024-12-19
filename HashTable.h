#pragma once
#include "main.h"

struct PairPoints
{
    XMFLOAT3 first;
    XMFLOAT3 second;

    PairPoints(XMFLOAT3 a, XMFLOAT3 b)
    {
        first = a;
        second = b;
    }
};

class HashTable 
{
private:
    struct Node 
    {
        XMFLOAT3 a, b;
        Node* next;

        Node(XMFLOAT3 a, XMFLOAT3 b) : a(a), b(b), next(nullptr) {}
    };

    Node** buckets;
    int size;

    size_t PointHash(const XMFLOAT3& point) const
    {
        const int prime1 = 73856093;
        const int prime2 = 19349663;
        const int prime3 = 83492791;
        int x = static_cast<int>(point.x * 1e6);
        int y = static_cast<int>(point.y * 1e6);
        int z = static_cast<int>(point.z * 1e6);

        return (x * prime1) ^ (y * prime2) ^ (z * prime3);
    }

    size_t TwoPointsHash(const PairPoints& points) const
    {
        const int prime = 31;
        size_t hash1 = PointHash(points.first);
        size_t hash2 = PointHash(points.second);

        return hash1 * prime + hash2;
    }

public:
    HashTable(int size) : size(size) 
    {
        buckets = new Node * [size];
        for (int i = 0; i < size; ++i) 
        {
            buckets[i] = nullptr;
        }
    }

    ~HashTable() 
    {
        for (int i = 0; i < size; ++i) 
        {
            Node* current = buckets[i];
            while (current != nullptr) 
            {
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
        delete[] buckets;
    }

    void insert(XMFLOAT3 a, XMFLOAT3 b)
    {
        unsigned int index = TwoPointsHash(PairPoints(a, b)) % size;
        Node* newNode = new Node(a, b);
        newNode->next = buckets[index];
        buckets[index] = newNode;
    }

    Node* search(XMFLOAT3 a, XMFLOAT3 b) const
    {
        unsigned int index = TwoPointsHash(PairPoints(a, b)) % size;
        Node* current = buckets[index];
        while (current != nullptr) 
        {
            BOOL equal = FALSE;
            equal |= current->a.x == a.x && current->a.y == a.y && current->a.z == a.z
                && current->b.x == b.x && current->b.y == b.y && current->b.z == b.z;
            equal |= current->a.x == b.x && current->a.y == b.y && current->a.z == b.z
                && current->b.x == a.x && current->b.y == a.y && current->b.z == a.z;
            if (equal)
            {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }
};