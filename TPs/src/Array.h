#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <vector>

template <typename T>
class Array
{
public:
  Array(int stride); // stride == taille des n-uplets
  ~Array();
  Array() = delete; //

  // accessors
  int size() const;         // retourne la taille utile
  int stride() const;       //retourne la taille des n-uplets
  const T* get(int i) const;// renvoie un pointeur sur le ième n-upplet.
  T* get(int i);
  int alloc_sz() const { return _alloc_sz; } // renvoie la taille allouée (en nb de n-uplets)

  // modifyers  
  void clear();                 // mets la taille utile à 0. 
  void reserve(int n);          // alloue (si besoin) la memoire pour n n-uplet, preserve les donnees existantes
  void destroy();               // desalloue la memoire
  void resize(int n, int val);  // en plus de ce que fait reserve, initialisation avec val de la partie utile non renseignee
  void add(const T* elt);             // ajoute un n-uplet

private:
  T* _data;                     // tableau des donnees
  int _stride;                  // taille des n-uplets
  int _sz, _alloc_sz;           // nb de n-uplet, nb max de n-uplets
};


template<typename T>
Array<T>::Array(int stride) :_stride(stride),_data(nullptr), _sz(0), _alloc_sz(0)
{
}

template<typename T>
void Array<T>::destroy()
{
  if (_data != nullptr)
  {
    delete[] _data;
    _data = nullptr;
  }
}

template<typename T>
Array<T>::~Array()
{
  destroy();
}

template<typename T>
void Array<T>::add(const T* elt)
{
  if (_alloc_sz == _sz)
    reserve(2*_sz + 1);

  for (int k = 0; k < _stride; ++k)
    _data[_sz*_stride + k] = elt[k];

  ++_sz;
}

template<typename T>
const T* Array<T>::get(int i) const
{
  return &_data[_stride*i];
}

template<typename T>
T* Array<T>::get(int i)
{
  return &_data[_stride*i];
}

template<typename T>
void Array<T>::clear()
{
  _sz = 0;
}

///
template<typename T>
void Array<T>::reserve(int n)
{
  if (n <= _alloc_sz) return;
  
  //  allocate new mem
  T* new_data = new T[n*_stride];
  //move exisitng data
  for (int i = 0; i < _sz*_stride; ++i)
    new_data[i] = _data[i];
  //delete old mem
  destroy();
  // reassign
  _data = new_data;
  _alloc_sz = n;
}

template<typename T>
void Array<T>::resize(int n, int val)
{
  reserve(n);
  if (n > _sz)
    for (int i = _sz * _stride; i < n*_stride; ++i)
      _data[i] = val;
  _sz = n;
}

template<typename T>
int Array<T>::size() const {
  return _sz;
}

template<typename T>
int Array<T>::stride() const
{
  return _stride;
}

#endif