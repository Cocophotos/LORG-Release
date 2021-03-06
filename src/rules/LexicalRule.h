// -*- mode: c++ -*-
#ifndef LEXICALRULE_H_
#define LEXICALRULE_H_

#include <iostream>
#include "AnnotatedRule.h"
#include "Types4Rules.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

class LexicalRule : public AnnotatedRule
{
public:
  LexicalRule();
  ~LexicalRule();

  LexicalRule(short l, int rhs0, const std::vector<lexical_proba_info>& probs);

  LexicalRule(short lhs_, int rhs_, const std::vector<double> lex_probs) :
    AnnotatedRule(lhs_), rhs0(rhs_), probabilities(lex_probs) {}

  /**
     \brief -> always true
   */
  inline bool is_lexical() const {return true;}
  /**
     \brief -> always false
   */
  inline bool is_unary() const {return false;}
  /**
     \brief -> always false
   */
  inline bool is_binary() const {return false;}



  /**
     \brief returns attribute rhs0
  */
  int get_rhs0() const;

    /**
       \brief set attribute rhs0
       \param r new value for rhs0
  */
  void set_rhs0(int r) {rhs0 = r;};


  /**
     \brief read access to annotated probabilities
     \param a annotation for lefthandside symbol
  */
  const double& get_probability(unsigned short a) const;

  /**
     \brief write access to annotated probabilities
     \param a annotation for lefthandside symbol
     \param value the new probability value
  */
  void   set_probability(unsigned short a, const double& value);
  void   set_probability(const double& value);

  unsigned get_num_annotations() const;

  int get_word() const;

  /**
     \brief read/write access to the 2d-vector of probabilities
     \todo should be private ?
  */
  const std::vector<double>& get_probability() const;
  std::vector<double>& get_probability();

  /**
     \brief Output operator
     \param out the ostream to write on
     \param rule the urule object to write
     \return the used ostream
  */
  friend std::ostream& operator<<(std::ostream& out, const LexicalRule& rule);

  void update_inside_annotations(std::vector<double>& up) const;

  void update_outside_annotations(const std::vector<double>& up,
                                  double& left) const;

  double update_outside_annotations_return_marginal(const std::vector<double>& up) const;

  /**
     \brief replace annotations with 0 if they're bel;ow threshold
     \param threshold the threshold
  */
  void remove_unlikely_annotations(const double& threshold);

  bool is_empty() const {return probabilities.empty();}

  /**
     \brief smooth probabilities *over all* lhs annotations
     \param alpha the small constant
  */
  void linear_smooth(const double& alpha);

  void weighted_smooth(const double& alpha, const std::vector<std::vector<double> > & weights);

  void generation_smooth(const std::vector<std::vector<std::vector<double> > >& weights);



  bool operator==(const LexicalRule&) const;
  bool operator<(const LexicalRule&) const;

private:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
      ar & boost::serialization::base_object<AnnotatedRule>(*this);
      ar & rhs0;
      ar & probabilities;
    }





protected:
  int rhs0;
  std::vector<double> probabilities ; ///< probabilities for a CFG rule  with annotation (no annotation on terminals)
};

inline
int LexicalRule::get_rhs0() const
{
  return rhs0;
}

inline
const double& LexicalRule::get_probability(unsigned short a) const
{
  return probabilities[a];
}

inline
void LexicalRule::set_probability(unsigned short a, const double& value)
{
  probabilities[a]=value;
}

inline
const std::vector<double>& LexicalRule::get_probability() const
{
  return probabilities;
}

inline
std::vector<double>& LexicalRule::get_probability()
{
  return probabilities;
}

inline
unsigned LexicalRule::get_num_annotations() const{

	return probabilities.size();
}

inline
int LexicalRule::get_word() const
{
  return get_rhs0();
}

#endif /*LEXICALRULE_H_*/
