#ifndef ChartCKY_hpp
#define ChartCKY_hpp

#include "ChartCKY.h"
#include "PCKYAllCell.hpp"

/* use parallel_for in opencells_apply_bottom_up and opencells_apply_top_down */
//#define WITH_PARALLEL_FOR

/* choices for the opencells_apply method */
// #define OPENCELLS_APPLY_TASK_GROUP
#define OPENCELLS_APPLY_PARALLEL_FOR
// #define OPENCELLS_APPLY_CONTINUATION_TASK
//#define OPENCELLS_APPLY_CHART_TASK

#ifdef USE_THREADS
template<class Types> unsigned ChartCKY<Types>::nbthreads;
#endif

// assume that words in sentence are in left to right direction (start in ascending direction)
template<class Types>
ChartCKY<Types>::ChartCKY(const std::vector< MyWord >& s,
                          unsigned grammar_size,
                          const std::vector<bracketing>& bs)
    : size(find_last_in_sentence(s)), sentence(s), brackets(bs)
{
  Cell::set_max_size(grammar_size);
  {
    //       BLOCKTIMING("ChartCKY<Types>::ChartCKY theCells.assign");
    nb_cells = (size*(size+1))/2;
    unsigned nb_edges = nb_cells*grammar_size;
    //       the_cells.assign(nbcells, protoCell);
    the_cells = (Cell*) new char[nb_cells*sizeof(Cell)];
    the_edges = (Edge*) new char[nb_edges*sizeof(Edge)];
    std::fill((char*)the_cells, (char*)(the_cells+nb_cells), 0);
    std::fill((char*)the_edges, (char*)(the_edges+nb_edges), 0);
  }
//     chart = new Cell * [size];

  Edge * edge = the_edges;
  for(unsigned i = 0; i < size; ++i) {

    //    std::cout << "i: " << i << std::endl;
    {
      //         BLOCKTIMING("ChartCKY<Types>::ChartCKY chart[i] = line_start;");
//         chart[i] = line_start;//new Cell[size-i];
    }
//     for(unsigned j = i; j < size;++j, edge+=grammar_size) {
//       //         BLOCKTIMING("ChartCKY<Types>::ChartCKY cell.init");
//       Cell& cell = access(i,j);
//       struct cell_close_helper
//       {
//         const bracketing& brackets;
//         cell_close_helper(const bracketing& b) : brackets(b) {}
//         bool operator()(const bracketing& other) const
//         {
//           return brackets.overlap(other);
//         }
//       };
    for(unsigned j = i; j < size;++j, edge+=grammar_size) {
      //         BLOCKTIMING("ChartCKY<Types>::ChartCKY cell.init");
      Cell& cell = access(i,j);
      bool close = brackets.end() != std::find_if(brackets.begin(),brackets.end(),
                                                  [&](const bracketing & other){return bracketing(i,i+j).overlap(other);}) ;
      cell.init(close, i, j, edge, i==0 and j==size-1);
    }
  }

  for (unsigned i = 0; i < sentence.size(); ++i)
  {
    //       BLOCKTIMING("ChartCKY<Types>::ChartCKY add_word");
    // todo: proper error handling
    if(access(sentence[i].get_start(), sentence[i].get_end()-1).is_closed())
      std::clog << "Problem in chart initialisation: brackets and tokenization are insconsistent." << std::endl;

    access(sentence[i].get_start(), sentence[i].get_end()-1).add_word(sentence[i]);
  }
//     std::cout << "Chart is built and intialised" << std::endl;
//     std::cout << *this << std::endl; std::cout.flush();
}




  template<class Types>
void
ChartCKY<Types>::opencells_apply_nothread( std::function<void(Cell &)> f)
{
  opencells_apply_bottom_up_nothread(f);
}

template<class Types>
void
ChartCKY<Types>::opencells_apply_bottom_up_nothread( std::function<void(Cell &)> f, unsigned min_span)
{

  unsigned sent_size = get_size();
  for (unsigned span = min_span; span < sent_size; ++span) {
    unsigned end_of_begin=sent_size-span;
    for (unsigned begin=0; begin < end_of_begin; ++begin) {
      unsigned end = begin + span ;

      //std::cout << "(" << begin << "," << end << ")" << std::endl;

      Cell& cell = access(begin,end);
      if(!cell.is_closed()) f(cell);
    }
  }
}

template<class Types>
void
ChartCKY<Types>::opencells_apply_top_down_nothread( std::function<void(Cell &)> f)
{
  unsigned sent_size=get_size();
  for (signed span = sent_size-1; span >= 0; --span) {
    unsigned end_of_begin=sent_size-span;

    for (unsigned begin=0; begin < end_of_begin; ++begin) {
      unsigned end = begin + span ;
      //std::cout << '(' << begin << ',' << end << ')' << std::endl;

      Cell& cell = access(begin,end);
      if(!cell.is_closed()) f(cell);
    }
  }
}

template<class Types>
void
ChartCKY<Types>::opencells_apply_top_down_nothread( std::function<void(const Cell &)> f) const
{
  unsigned sent_size=get_size();
  for (signed span = sent_size-1; span >= 0; --span) {
    unsigned end_of_begin=sent_size-span;

    for (unsigned begin=0; begin < end_of_begin; ++begin) {
      unsigned end = begin + span ;
      //std::cout << '(' << begin << ',' << end << ')' << std::endl;

      const Cell& cell = access(begin,end);
      if(!cell.is_closed()) f(cell);
    }
  }
}

#ifndef USE_THREADS

template<class Types>
inline
void
ChartCKY<Types>::opencells_apply( std::function<void(Cell &)> f)
{
  opencells_apply_nothread(f);
}

template<class Types>
inline
void
ChartCKY<Types>::opencells_apply_bottom_up( std::function<void(Cell &)> f, unsigned min_span)
{
  opencells_apply_bottom_up_nothread(f, min_span);
}

template<class Types>
inline
void
ChartCKY<Types>::opencells_apply_top_down( std::function<void(Cell &)> f)
{
  opencells_apply_bottom_up_nothread(f);
}



#else // USE_THREADS defined

#ifdef WITH_PARALLEL_FOR
template<class Types>
void
ChartCKY<Types>::opencells_apply_bottom_up( std::function<void(Cell &)> f, unsigned min_span )
{
  unsigned sent_size = get_size();
  for (unsigned span = min_span; span < sent_size; ++span) {
    unsigned end_of_begin=sent_size-span;
    tbb::parallel_for(tbb::blocked_range<unsigned>(0, end_of_begin),
                      [this,span,&f](const tbb::blocked_range<unsigned>& r){
                        for (unsigned begin = r.begin(); begin < r.end(); ++begin) {
                          unsigned end = begin + span ;
                          Cell& cell = this->access(begin,end);
                          if(!cell.is_closed()) f(cell);
                        }
                      }
    );
  }
}
template<class Types>
void
ChartCKY<Types>::opencells_apply_top_down( std::function<void(Cell &)> f)
{
  unsigned sent_size=get_size();
  for (signed span = sent_size-1; span >= 0; --span) {
    unsigned end_of_begin=sent_size-span;

    tbb::parallel_for(tbb::blocked_range<unsigned>(0, end_of_begin),
                      [this,span,&f](const tbb::blocked_range<unsigned>& r){
                        for (unsigned begin = r.begin(); begin < r.end(); ++begin) {
                          unsigned end = begin + span ;
                          //std::cout << '(' << begin << ',' << end << ')' << std::endl;

                          Cell& cell = this->access(begin,end);
                          if(!cell.is_closed()) f(cell);
                        }
                      }
    );
  }
}

template<class Types>
void
ChartCKY<Types>::opencells_apply( std::function<void(Cell &)> f)
{
  tbb::parallel_for(tbb::blocked_range<Cell*>(the_cells, the_cells + nb_cells),
                    [&f](const tbb::blocked_range<Cell*>& r)
                    {
                      for (auto cell = r.begin(); cell < r.end(); ++cell) {
                        if(! cell->is_closed()) f(*cell);
                      }
                    }
                    );
}



#else // WITH_PARALLEL_FOR not defined

class ChartTask: public tbb::task {
  const std::function<void()> action ;
public:
  //   virtual ~ChartTask() {(std::cout << "destruction of " << this << std::endl).flush();}
  ChartTask(std::function<void()> _action)
  : action(_action)
  { /*std::cout << "initialisation" << endl;*/}

  tbb::task* successor[2]; // always NULL in constructor ?

  task* execute() {
    __TBB_ASSERT( ref_count()==0, NULL );
    action();
    for( int k=0; k<2; ++k )
      if( tbb::task* t = successor[k] ) {
        if( t->decrement_ref_count()==0 )
          spawn( *t );
      }
      return NULL;
  }
  };


  #ifdef OPENCELLS_APPLY_PARALLEL_FOR
  template<class Types>
  void
  ChartCKY<Types>::opencells_apply( std::function<void(Cell &)> f)
  {
    tbb::parallel_for(tbb::blocked_range<Cell *>(the_cells, the_cells+nb_cells),
                      [&f](const tbb::blocked_range<Cell *>& r){
//                         std::cerr << "blockedrange="<< (r.end() - r.begin()) << std::endl;
                        for (auto cell = r.begin(); cell < r.end(); ++cell) {
                          if(!cell->is_closed()) f(*cell);
                        }
                      }
    );
  }
  #endif

  #ifdef OPENCELLS_APPLY_CONTINUATION_TASK
  #include <tbb/atomic.h>
  using tbb::atomic;

  template<typename Cell>
  class ParallelTask: public tbb::task {
    const std::function<void(Cell &)> action ;
    atomic<Cell*> & it  ;
    Cell* end ;
    tbb::task * waiter;

  public:
    ParallelTask(std::function<void(Cell &)> _action, atomic<Cell *> & _it, Cell * _end, tbb::task * _waiter)
    : action(_action), it(_it), end(_end), waiter(_waiter)
    {
      //     std::cout << "ParallelTask it = " << _it << std::endl;
    }
    task* execute() {
      __TBB_ASSERT( ref_count()==0, NULL );
      Cell * oldit, * newit;
      do {
        oldit = it ; newit = oldit;
        if (newit!=end) ++newit;
      } while(it.compare_and_swap(newit,oldit) != oldit);

      if (oldit != end) {
        //       std::cout << "cell : " << it << " ? " << end << std::endl ;
        if (not oldit->is_closed()) action(*oldit);
        recycle_as_continuation();
        return this;
      } else {
        waiter->decrement_ref_count();
        return NULL;
      }
    }
  };

  template<class Types>
  void
  ChartCKY<Types>::opencells_apply( std::function<void(Cell &)> f)
  {
    atomic<Cell *> it; it = the_cells.data();
    tbb::task_list seeds;
    tbb::task * waiter = new( tbb::task::allocate_root() ) tbb::empty_task;
    for (signed t=0; t< nbthreads; ++t) {
      ParallelTask<Cell> & task = *new (tbb::task::allocate_root()) ParallelTask<Cell>(f, it, the_cells.data()+the_cells.size(), waiter);
      task.set_ref_count(2);
      seeds.push_back(task);
      waiter->increment_ref_count();
    }
    waiter->increment_ref_count();
    waiter->spawn_and_wait_for_all(seeds);
    tbb::task::destroy(*waiter);
  }
  #endif


  #ifdef OPENCELLS_APPLY_TASK_GROUP

  #include "tbb/task_group.h"

  template<class Types>
  void
  ChartCKY<Types>::opencells_apply( std::function<void(Cell &)> f)
  {
    tbb::task_group g;
    for(auto & cell: the_cells) {
      if(!cell.is_closed()) g.run([&cell,&f](){f(cell);});
    }
    g.wait();
  }
  #endif


  #ifdef OPENCELLS_APPLY_CHART_TASK
  template<class Types>
  void
  ChartCKY<Types>::opencells_apply( std::function<void(Cell &)> f)
  {
    tbb::task * waiter = new( tbb::task::allocate_root() ) tbb::empty_task;

    tbb::task_list seeds;
    unsigned count = 0;

    for(auto & cell: the_cells) {
      if(not cell.is_closed())
      {
        ChartTask * t = new( tbb::task::allocate_root() ) ChartTask([&cell,&f](){f(cell);});
        t->successor[0] = waiter;
        t->successor[1] = NULL;
        t->set_ref_count(2);
        seeds.push_back(*t);
        ++count;
      }
    }

    waiter->set_ref_count(count +1);
    // Wait for all tasks to complete.
    waiter->spawn_and_wait_for_all(seeds);
    tbb::task::destroy(*waiter);
  }
  #endif


  template<class Types>
  void
  ChartCKY<Types>::opencells_apply_bottom_up( std::function<void(Cell &)> f, unsigned min_span )
  {
    signed min_s = min_span;
    signed sent_size = get_size();
    if (min_s>=sent_size) return;

    tbb::task * waiter = new( tbb::task::allocate_root() ) tbb::empty_task;
    ChartTask* x[sent_size][sent_size];
    for (signed span = sent_size-1; span>=min_s; --span) {
      unsigned end_of_begin=sent_size-span;
      for (unsigned begin=0; begin < end_of_begin; ++begin) {
        unsigned end = begin + span ;
        Cell * cell = & this->access(begin,end);
        if (cell->is_closed())
          x[span][begin] = new( tbb::task::allocate_root() ) ChartTask([](){});
        else
          x[span][begin] = new( tbb::task::allocate_root() ) ChartTask([cell,&f](){f(*cell);});

        // successor[0] = successor up-left, successor[1] = successor up
          x[span][begin]->successor[0] = span<sent_size-1 && begin>0              ? x[span+1][begin-1] : NULL;
          x[span][begin]->successor[1] = span<sent_size-1 && begin<end_of_begin-1 ? x[span+1][begin  ] : NULL;
          x[span][begin]->set_ref_count((span>min_s)*2);
          //       (std::cout << "created ("<< span << "," << begin << "," << end << " | " << x[span][begin]->successor[0] << "," << x[span][begin]->successor[1] << ")\n").flush();
      }
    }

    x[sent_size-1][0]->successor[0] = waiter;
    //   (std::cout << "waiter("<< span << "," << begin << "," << end << " | " << x[span][begin]->successor[0] << "," << x[span][begin]->successor[1] << ")\n").flush();
    waiter->set_ref_count(2);
    tbb::task_list seeds;
    for (signed begin=0; begin<sent_size-min_s; ++begin)
      seeds.push_back(*x[min_span][begin]);
    // Wait for all tasks to complete.
      waiter->spawn_and_wait_for_all(seeds);
      tbb::task::destroy(*waiter);
  }

  template<class Types>
  void
  ChartCKY<Types>::opencells_apply_top_down( std::function<void(Cell &)> f )
  {
    signed sent_size = get_size();
    tbb::task * waiter = new( tbb::task::allocate_root() ) tbb::empty_task;
    ChartTask* x[sent_size][sent_size];
    for (signed span = 0; span<sent_size; ++span) {
      unsigned end_of_begin=sent_size-span-1;
      for (unsigned begin=0; begin <= end_of_begin; ++begin) {
        unsigned end = begin + span ;

        Cell * cell = & this->access(begin,end);
        if (cell->is_closed())
          x[span][begin] = new( tbb::task::allocate_root() ) ChartTask([](){});
        else
          x[span][begin] = new( tbb::task::allocate_root() ) ChartTask([cell,&f](){f(*cell);});

        x[span][begin]->successor[0] = span>0 ? x[span-1][begin] : waiter;
        x[span][begin]->successor[1] = span>0 ? x[span-1][begin+1] : NULL;
        x[span][begin]->set_ref_count((begin<end_of_begin) + (begin>0));

        //       (std::cout << "created ("<< span << "," << begin << "," << end << " | " << x[span][begin]->successor[0] << "," << x[span][begin]->successor[1] << ")\n").flush();
      }
    }

    waiter->set_ref_count(sent_size+1);
    waiter->spawn_and_wait_for_all(*x[sent_size-1][0]);
    tbb::task::destroy(*waiter);
  }

  #endif // WITH_PARALLEL_FOR
  #endif // USE_THREADS



  template<class Types>
  std::ostream & operator<<(std::ostream & out, const ChartCKY<Types> & chart) { return chart.to_stream(out) ; }

  template<class Types>
  std::ostream & ChartCKY<Types>::to_stream(std::ostream & s) const {
    s << "(begin chart:"<< this << ")" << std::endl;
    opencells_apply_top_down_nothread( [&s](const Cell & cell){
      s << "(span " << cell.get_end()-cell.get_begin()+1 << ", begin " << cell.get_begin() << ")" << std::endl;
      s << cell << std::endl;
    } );
    s << "(end   chart "<< this << ")" << std::endl;
    return s;
  }

#include <sstream>

  template<class Types>
  std::string ChartCKY<Types>::toString() const {
    std::string ret;
    std::ostringstream stream(ret);
    opencells_apply_top_down_nothread( [&stream](const Cell & cell){

      stream << "(span " << cell.get_end()-cell.get_begin()+1 << ", begin " << cell.get_begin() << ")" << std::endl;
      stream << cell << std::endl;
    } );
    return ret;
  }

  //#include "utils/SymbolTable.h"

  template<class Types>
  inline
  unsigned ChartCKY<Types>::get_size() const
  {
    return size;
  }

  template<class Types>
  inline
  typename Types::Cell& ChartCKY<Types>::access(unsigned start, unsigned end)
  {
    //   assert(start <= end);
    //   assert(end < size);
    //     return chart[start][end-start];
//     std::cout << "access("<<start<<","<<end<<") = "<<start + ( (end-start)*(2*size - (end-start) + 1))/2 << std::endl; std::cout.flush();
    return the_cells[start + ( (end-start)*(2*size - (end-start) + 1))/2 ];
  }

  template<class Types>
  inline
  const typename Types::Cell& ChartCKY<Types>::access(unsigned start, unsigned end) const
  {
    //   assert(start <= end);
    //   assert(end < size);
    //     return chart[start][end-start];
    return the_cells[start + ( (end-start)*(2*size - (end-start) + 1))/2 ];
  }

  template<class Types>
  ChartCKY<Types>::~ChartCKY()
  {
    opencells_apply([](Cell&cell){cell.Cell::~Cell();});
    // for (unsigned i=0; i<nb_cells * Cell::get_max_size(); ++i) {if (not the_edges[i].is_closed()) the_edges[i].Edge::~Edge();}
    delete [] (char*) the_cells;
    delete [] (char*) the_edges;

//     for(unsigned i = 0; i < size; ++i)
//       delete[] chart[i];
//     delete[] chart;
  }


  template<class Types>
  typename Types::Cell& ChartCKY<Types>::get_root()
  {
    return access(0,size-1);
  }

  template<class Types>
  const typename Types::Cell& ChartCKY<Types>::get_root() const
  {
    return access(0,size-1);
  }

  template<class Types>
  PtbPsTree* ChartCKY<Types>::get_best_tree(int start_symbol, unsigned k) const
  {
    PtbPsTree* tree = NULL;

    const Cell & root_cell = this->get_root();

    if (!root_cell.is_closed() && root_cell.exists_edge(start_symbol)) {
      tree = root_cell.get_edge(start_symbol).to_ptbpstree(start_symbol, k);
    }

    return tree;
  }

  //score at root
  template<class Types>
  double ChartCKY<Types>::get_score(int symbol, unsigned k) const
  {
    return get_root().get_edge(symbol).get_prob_model().get(k).probability;
  }


  template<class Types>
  void ChartCKY<Types>::init(const std::vector< MyWord >& sentence)
  {
    //iterate through all the words in the sentence
    for(typename std::vector<MyWord>::const_iterator w_itr(sentence.begin());
        w_itr != sentence.end(); ++w_itr)
    {
      Cell& cell = this->access(w_itr->get_start(), w_itr->get_end() -1);
      cell.add_word(*w_itr);
    }
  }

  template<class MyWord>
  unsigned find_last_in_sentence(const std::vector< MyWord >& s)
  {
    unsigned res = 0;
    for (typename std::vector< MyWord >::const_iterator i(s.begin()); i != s.end(); ++i)
    {
      if ((unsigned) i->get_end() > res) res = (unsigned) i->get_end();
    }
    return res;
  }


  template<class Types>
  void ChartCKY<Types>::reset_probabilities()
  {
    for(unsigned i = 0; i < size; ++i)
      for(unsigned j = i; j < size; ++j) {
        //      std::cout << "(" << i << "," << j << ")" << std::endl;
        access(i,j).reset_probabilities();
      }
  }


  template<class Types>
  bool ChartCKY<Types>::has_solution(int symb, unsigned i) const
  {
    //  std::cout << SymbolTable::instance_nt().translate(symb) << std::endl;
    return get_root().get_edge(symb).has_solution(i);
  }

  template<class Types>
  void ChartCKY<Types>::clear()
  {
    for(unsigned i = 0; i < size; ++i)
      for(unsigned j = i; j < size; ++j) {
        access(i,j).clear();
      }
  }

  template<class Types>
  void ChartCKY<Types>::prepare_retry()
  {
    this->clear();
    this->reset_probabilities();

    for(unsigned i = 0; i < size; ++i) {
      // j == 0 -> word position
      Cell& cell = access(i,i); //the_chart[i]; //chart[i][0];
      cell.reinit(false);
      cell.add_word(sentence[i]);

      // regular chart cells
      for(unsigned j = 1; j < size-i;++j) {
        bool close = brackets.end() != std::find_if(brackets.begin(),brackets.end(),
                                                    [&](const bracketing & other){return bracketing(i,i+j).overlap(other);}) ;
        access(i,i+j)/*chart[i][j]*/.reinit(close);
      }
    }
  }

  template <class Types>
  bool ChartCKY<Types>::is_valid(int start_symbol) const
  {
    return !get_root().is_closed() && get_root().exists_edge(start_symbol);
  }


  template<class Types>
  SET<const typename ChartCKY<Types>::Edge*> ChartCKY<Types>::get_rules_best_solution(int start_symbol) const
  {
    SET<const typename ChartCKY<Types>::Edge*> result;

    const Cell & root_cell = this->get_root();

    if (!root_cell.is_closed() && root_cell.exists_edge(start_symbol)) {
      root_cell.get_edge(start_symbol).to_set(result);
    }

    return result;
  }


template<class Types>
void ChartCKY<Types>::update_relaxations(bool simplify, const MAP<int,MAP<int, MAP<int,double>>>& lambda, const std::unordered_map<int,int>& simple_map)
{
  for (const auto& k1 : lambda)
  {
    for (const auto& k2: k1.second)
    {
      auto& cell = access(k1.first, k2.first);
      cell.update_relaxations(simplify, k2.second, simple_map);
    }
  }
}


#endif
