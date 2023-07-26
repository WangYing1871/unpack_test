//--------------------------------Stamp-------------------------------
//^-^ Author: Zhi Heng            Email: wy187110@mail.ustc.edu.cn     
//^-^ Time: 2023-07-25 09:56:39   Posi: Hefei
//^-^ File: base.hpp
//--------------------------------------------------------------------
#define info_out(X) std::cout<<"==> "<<__LINE__<<" "<<#X<<" |"<<(X)<<"|\n"
#include <array>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <type_traits>

struct APD{  //TODO
  static std::string const& name() {return _name;}

private:
  static std::string const _name;
};
std::string const APD::_name = "APD";

template <class _tp, class _cate_t>
struct channal_base{
  typedef _tp tag_t;
  typedef _cate_t categeoy_t;

  channal_base() = default;
  channal_base(_tp const& _0):_tag(_0) {}
  channal_base(_tp const& _0, _cate_t const& _1):_tag(_0),_category(_1) {}
  virtual ~channal_base() noexcept = default;

  inline void set_tag(_tp const& _0) {_tag = _0;}
  inline void set_cate(_cate_t const& _0) {_category = _0;}
  inline _tp get_tag() const {return _tag;}
  inline _cate_t get_category() const {return _category;}
protected:
  _tp _tag = _tp{};
  _cate_t _category = _cate_t{};
};

template <class _data_t>
struct channal : public channal_base<uint32_t,APD>{
  typedef _data_t data_value;
  typedef channal_base base_t;
//private:
public:
  channal() = default;
  template <class _tp>
  requires (std::is_convertible_v<_tp,uint32_t>)
  channal(_tp const& _0): base_t(static_cast<uint32_t>(_0)) {}
  ~channal() noexcept = default;
  

  _data_t _data_set=_data_t{};

};

template <class _tp>
struct FEE_board_base{
  void name(_tp const& _p0) {_name = _p0;}
  _tp name() const {return _name;}

protected:
  _tp _name;
  uint32_t _freq;
  std::pair<double,double> _wk_volt{0.,0.};
  std::pair<double,double> _wk_curr{0.,0.};
};

template <class _key_t, class _channal_t>
requires requires {typename _channal_t::tag_t;}
struct FEE_board : public FEE_board_base<std::string>{


  typedef std::pair<bool,_channal_t> channal_t;
  typedef typename _channal_t::tag_t channal_tag_t;
  _channal_t get_channal(_key_t const& _p0) const {return _channal_t{};}
  void remove_channal(_key_t const& _p0) {}
  void enable_channal(_key_t const& _p0, bool _p1=false) {}

  inline void add_channal(_key_t const& _p0, bool _p1=true){
    _channals.insert(std::make_pair(_p0,std::make_pair(_p1,_channal_t{}))); }
  inline void add_channal(_key_t const& _0, _channal_t const& _1){
    _channals[_0] = std::make_pair(true,_1); }

  template <class _tp, class _invoker, class... _args> //TODO
  void add_channal(_tp const& _0, _invoker const& _1
      ,_args&&... _params){
    if constexpr(std::is_convertible_v<_tp,channal_tag_t> && 
        std::is_same_v<_invoker,channals_table_t> && sizeof...(_args)==0){
      _channal_t cl{static_cast<channal_tag_t>(_0)}; auto key = std::invoke(_1,_0);
      this->add_channal(key,cl);
    }else{
      _channal_t cl{static_cast<channal_tag_t>(_0)}; 
      auto key = std::invoke(_1,_0,std::forward<_args>(_params)...);
      this->add_channal(key,cl);
    } }

private:
public:
  std::map<_key_t,channal_t> _channals; //FIXME |shared_ptr| shouble be used ?
  std::function<_key_t(channal_tag_t const&)> channals_table;

  typedef decltype(channals_table) channals_table_t;
  inline void set_ctb(channals_table_t const& _p0){ channals_table = _p0;}
  
#ifdef VLS_DEBUG
  inline uint32_t channals_size() const {return _channals.size();}
  std::map<_key_t,channal_t> const& channals() const {return _channals;}
#endif
};

template <class _tp>
struct device_base{
  _tp name() const {return _name;}
  void name(_tp const& _a) { _name = _a;}
protected:
  _tp _name;
};

template <class _key_t, std::size_t NN=1024>
struct vl_cr_test_device : device_base<std::string>{
  //TODO
  typedef std::array<uint16_t,NN> channal_wave_t;
  typedef channal<channal_wave_t> channal_adc_t;
  typedef FEE_board<std::string,channal_adc_t> FEE_board_t;
  typedef FEE_board_base<std::string> value_type;

  void add_board(_key_t const _0, value_type* _1){
    _FEE_boards.insert(std::make_pair(_0,_1)); }

  std::map<_key_t,FEE_board_base<std::string>*>const& 
  FEE_boards() const {return _FEE_boards;}

  friend class unname_aa; //for unpack the binary file.

protected:
  std::map<_key_t,FEE_board_base<std::string>*> _FEE_boards;
};


//---------------------------------------------------------------------
//class event{
struct unname_aa{
  uint32_t Id;
  float timestamp;

  typedef vl_cr_test_device<std::string,512> device_t;
  device_t* _device;

  void set_device(device_t& _0) {_device = std::addressof(_0);}

  std::size_t get_boards() const {return _device->_FEE_boards.size();}

  void prase(){
  }

  


};

template <class _tp=unsigned char>
struct pack_base{
  typedef pack_base self_t;
  typedef _tp unit_type;

  pack_base() = default;
  virtual ~pack_base() noexcept = default;
  virtual bool valid() const=0;
  virtual uint32_t size() const=0;
  virtual void fill(void const*);
};

//template <class _tp>
//struct pack_bl : public pack_base<_tp> {};
