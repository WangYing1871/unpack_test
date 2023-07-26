//--------------------------------Stamp-------------------------------
//^-^ Author: Zhi Heng            Email: wy187110@mail.ustc.edu.cn     
//^-^ Time: 2023-07-21 21:24:52   Posi: Hefei
//^-^ File: read_rt.cxx
//--------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <bitset>
#include <type_traits>


template <class _tp=unsigned char>
struct pack_base{
  typedef pack_base self_t;
  typedef _tp unit_type;
  pack_base() {}
  ~pack_base() noexcept {}

  virtual bool is_valid() const = 0;
  virtual unsigned int size() const = 0;

  virtual void fill(void const*);

};
template <class _tp>
struct pack_bl : public pack_base<_tp> {};

template <class _tp, template <class, class...> class _cont_tt=std::vector>
struct pack_bb : public pack_base<_tp>{
  _cont_tt<pack_base<_tp>*> sub_packs;
};



struct evt_head_t : public pack_bl<unsigned char>{
  constexpr static float freq = 1.2E+8;
  typedef typename pack_base::unit_type unit_type;

  constexpr static unit_type _MIN_ = (std::numeric_limits<unit_type>::min)();
  constexpr static unit_type _MAX_ = (std::numeric_limits<unit_type>::max)();
private:
  unit_type head_tag = _MIN_;
  unit_type soe_eoe_packsize[2] = {_MIN_,_MIN_};
  unit_type FEid = _MIN_;
  unit_type time_stamp[6];
  unit_type event_count[4];
  unit_type channal_count = _MIN_;
  unit_type reserved = _MIN_;
  unit_type crc32[4];

  evt_head_t(){

  }

  friend struct bits_viewer;


public:
  virtual bool is_valid() const{
    return head_tag==0x5A
      and (std::bitset<8>(soe_eoe_packsize[0]).test(6)
        and !std::bitset<8>(soe_eoe_packsize[1]).test(5))
      and size()==20; }
  virtual unsigned int size() const{
    unsigned short tmp = (std::numeric_limits<unsigned short>::min)();
    tmp |= ((unsigned short)soe_eoe_packsize[0])<<8;
    tmp |= ((unsigned short)soe_eoe_packsize[1]);
    return (unsigned int)tmp; }
  virtual void fill(void const* _aptr){
    if (!_aptr) return;
    auto const* _ptr = reinterpret_cast<unit_type const*>(_aptr);
    head_tag = *_ptr++;
    soe_eoe_packsize[0]=*_ptr++, soe_eoe_packsize[1]=*_ptr++;
    FEid = *_ptr++;
    for(auto&& x : time_stamp) x=*_ptr++;
    for(auto&& x : event_count) x=*_ptr++;
    channal_count = *_ptr++;
    reserved = *_ptr++;
    for(auto&& x : crc32) x=*_ptr++; }
  void fill(FILE* _aptr){
    //TODO
  }
};

struct evt_tail_t : public pack_bl<unsigned char>{
  typedef typename pack_base::unit_type unit_type;
private:
  unit_type head_tag;
  unit_type soe_eoe_packsize[2];
  unit_type 
 
  
};

struct evt_body_t : public pack_bl<unsigned char>{



};


int main(int argc, char* argv[]){
  //evt_head_t opt_aa;

  return 0;
}
