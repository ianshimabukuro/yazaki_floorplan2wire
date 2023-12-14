#pragma once
#include<map>
#include<vector>
#include<algorithm>

namespace ewd
{
    template<typename T>
    using interval = std::pair<T,T>;
    
    template<typename T>
    void intervals_union(std::vector<interval<T>>& intvs)
    {
        sort(intvs.begin(), intvs.end(), [](const interval<T>& t1, const interval<T>& t2)->bool {return t1.first < t2.first;});
        auto MAX = [](const T& t1, const T& t2)-> T { return (t1 < t2) ? (t2) : t1;};
        size_t i=0;
        while (i+1 < intvs.size())
        {
            if(intvs[i].second < intvs[i+1].first)
                i++;
            else 
            {
                intvs[i].second = MAX(intvs[i].second, intvs[i+1].second);
                intvs.erase(intvs.begin() + i + 1);
            }
        }
    }

    template <typename T>
    void intervals_exclude(std::vector<interval<T>> &priority, std::vector<interval<T>> &inferior)
    {
        for (size_t i = 0; i < priority.size(); i++)
        {
            for (size_t j = 0; j < inferior.size();)
            {
                T l1 = inferior[j].first;
                T r1 = (priority[i].first <= inferior[j].second) ? priority[i].first : inferior[j].second;
                T l2 = (inferior[j].first >= priority[i].second) ? inferior[j].first : priority[i].second;
                T r2 = inferior[j].second;
                std::vector<interval<T>> newintvs;
                if (l1 < r1)
                    newintvs.push_back(std::make_pair(l1, r1));
                if (l2 < r2)
                    newintvs.push_back(std::make_pair(l2, r2));
                inferior.erase(inferior.begin() + j);
                inferior.insert(inferior.begin() + j, newintvs.begin(), newintvs.end());
                if (!newintvs.empty())
                    j++;
            }
        }
    }

} 