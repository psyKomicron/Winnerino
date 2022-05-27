#pragma once
#include "pch.h"

using namespace std;

namespace winrt::Winnerino::Sorting
{
    template<class T>
    class QuickSort
    {
    public:
        QuickSort() = default;

        void Sort(std::vector<T>* vect, const int64_t& low, const int64_t& high)
        {
            if (low < 0)
            {
                throw hresult_out_of_bounds(L"low");
            }
            if (high >= (int64_t)vect->size())
            {
                throw hresult_out_of_bounds(L"high");
            }

            if (low < high)
            {
                size_t partitionIndex = Partition(vect, low, high);

                Sort(vect, low, partitionIndex - 1);
                Sort(vect, partitionIndex + 1, high);
            }
        };

        void Sort(std::vector<T>* vect, const int64_t& low, const int64_t& high, const std::function<int8_t(const T& base, const T& other)>& comparer)
        {
            if (low < 0)
            {
                throw hresult_out_of_bounds(L"low");
            }
            if (high >= (int64_t)vect->size())
            {
                throw hresult_out_of_bounds(L"high");
            }

            if (low < high)
            {
                size_t partitionIndex = Partition(vect, low, high, comparer);

                Sort(vect, low, partitionIndex - 1, comparer);
                Sort(vect, partitionIndex + 1, high, comparer);
            }
        }

    private:
        inline void Swap(vector<T>* vect, int64_t const& i, int64_t const& j)
        {
#ifdef _DEBUG
            if (i >= (int64_t)vect->size())
            {
                throw hresult_out_of_bounds(L"i");
            }
            if (j >= (int64_t)vect->size())
            {
                throw hresult_out_of_bounds(L"j");
            }
#endif 

            T swapLow = vect->operator[](i);
            T swapHigh = vect->operator[](j);
            vect->operator[](i) = swapHigh;
            vect->operator[](j) = swapLow;
        }

        int64_t Partition(std::vector<T>* vect, int64_t low, int64_t high)
        {
            if (low < 0)
            {
                throw hresult_out_of_bounds(L"low");
            }
            if (high >= (int64_t)vect->size())
            {
                throw hresult_out_of_bounds(L"high");
            }

            T pivot = vect->operator[](high);
            int64_t i = low - 1;
            for (int64_t j = low; j <= high - 1; j++)
            {
                if (vect->operator[](j).Compare(pivot) < 0)
                {
                    i++;
                    Swap(vect, i, j);
                }
            }
            Swap(vect, i + 1, high);
            return i + 1;
        };

        int64_t Partition(std::vector<T>* vect, int64_t low, int64_t high, const std::function<int8_t(const T&, const T&)>& comparer)
        {
            if (low < 0)
            {
                throw hresult_out_of_bounds(L"low");
            }
            if (high >= (int64_t)vect->size())
            {
                throw hresult_out_of_bounds(L"high");
            }

            T pivot = vect->operator[](high);
            int64_t i = low - 1;
            for (int64_t j = low; j <= high - 1; j++)
            {
                if (comparer(vect->operator[](j), pivot) < 0)
                {
                    i++;
                    Swap(vect, i, j);
                }
            }
            Swap(vect, i + 1, high);
            return i + 1;
        }
    };
}

