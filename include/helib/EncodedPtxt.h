/* Copyright (C) 2012-2020 IBM Corp.
 * This program is Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. See accompanying LICENSE file.
 */
#ifndef HELIB_ENCODED_PTXT_H
#define HELIB_ENCODED_PTXT_H

#include <helib/DoubleCRT.h>
#include <helib/norms.h>

namespace helib {





class EncodedPtxt_BGV {

private:
  zzX poly;
  long ptxtSpace; 
  const Context& context;


public:

  const zzX& getPoly() const { return poly; }
  long getPtxtSpace() const { return ptxtSpace; }
  const Context& getContext() const { return context; }

  EncodedPtxt_BGV(const zzX& poly_, long ptxtSpace_, const Context& context_)
    : poly(poly_), ptxtSpace(ptxtSpace_), context(context_) { }

};

class EncodedPtxt_CKKS {

private:
  zzX poly;
  double mag, scale, err;
  const Context& context;

public:

  const zzX& getPoly() const { return poly; }
  double getMag() const { return mag; }
  double getScale() const { return scale; }
  double getErr() const { return err; }
  const Context& getContext() const { return context; }

  EncodedPtxt_CKKS(const zzX& poly_, double mag_, double scale_, double err_,
                   const Context& context_)
    : poly(poly_), mag(mag_), scale(scale_), err(err_), context(context_) { }

};


/*

The following is a lot of "boilerplate" code that 
gives a type-safe union of the above two EncodedPtxt types.

The idea is that an EncodedPtxt object eptxt contains
either an EncodedPtxt_BGV object, an EncodedPtxt_CKKS object,
or nothing at all.

* eptxt.isBGV() return true if it contains an EncodedPtxt_BGV object 

* eptxt.isCKKS() return true if it contains an EncodedPtxt_CKKS object 

* eptxt.getBGV() returns a read-only reference to the contained
     EncodedPtxt_BGV object (or throws a std::bad_cast exception)

* eptxt.getCKKS() returns a read-only reference to the contained
     EncodedPtxt_CKKS object (or throws a std::bad_cast exception)

* eptxt.resetBGV(poly, ptxtSpace, context) replaces the contents 
     of eptxt with a new EncodedPtxt_BGV object

* eptxt.resetCKKS(poly, mag, scale, err, context) replaces the contents 
     of eptxt with a new EncodedPtxt_CKKS object

* eptxt.reset() empties the contents

The default contructor creates an empty container.
Copy contructors and assignmemnt operators are available,
   and provide "deep" copy semantics

*/


class EncodedPtxt_base {
public:

  virtual ~EncodedPtxt_base() {}

  virtual EncodedPtxt_base* clone() const = 0;
  // makes this usable with cloned_ptr

  virtual bool isBGV() const { return false; }
  virtual bool isCKKS() const { return false; }

  virtual const EncodedPtxt_BGV& getBGV() const { throw std::bad_cast(); }
  virtual const EncodedPtxt_CKKS& getCKKS() const { throw std::bad_cast(); }

};

class EncodedPtxt_derived_BGV : public EncodedPtxt_base, 
                                public EncodedPtxt_BGV {
public:

  virtual EncodedPtxt_base* clone() const override 
  { return new EncodedPtxt_derived_BGV(*this); }

  virtual bool isBGV() const override { return true; }

  virtual const EncodedPtxt_BGV& getBGV() const override { return *this; }

  using EncodedPtxt_BGV::EncodedPtxt_BGV;
};


class EncodedPtxt_derived_CKKS : public EncodedPtxt_base, 
                                public EncodedPtxt_CKKS {
public:

  virtual EncodedPtxt_base* clone() const override 
  { return new EncodedPtxt_derived_CKKS(*this); }

  virtual bool isCKKS() const override { return true; }

  virtual const EncodedPtxt_CKKS& getCKKS() const override { return *this; }

  using EncodedPtxt_CKKS::EncodedPtxt_CKKS;
};


class EncodedPtxt {
  cloned_ptr<EncodedPtxt_base> rep;

public:

  bool isBGV() const { return !rep.null() && rep->isBGV(); }
  bool isCKKS() const { return !rep.null() && rep->isCKKS(); }

  const EncodedPtxt_BGV& getBGV() const 
  { if (rep.null()) throw std::bad_cast(); return rep->getBGV(); }

  const EncodedPtxt_CKKS& getCKKS() const 
  { if (rep.null()) throw std::bad_cast(); return rep->getCKKS(); }

  void resetBGV(const zzX& poly, long ptxtSpace, const Context& context) 
  { 
    rep.set_ptr(new EncodedPtxt_derived_BGV(poly, ptxtSpace, context));
  }

  void resetCKKS(const zzX& poly, double mag, double scale, double err, const Context& context) 
  { 
    rep.set_ptr(new EncodedPtxt_derived_CKKS(poly, mag, scale, err, context));
  }

  void reset() 
  {
    rep.set_ptr(0);
  }

};





//=========================================================
//
// "fat" encodings...same as above, but with DCRT's instead.


class FatEncodedPtxt_BGV {

private:
  DoubleCRT dcrt;
  long ptxtSpace; 
  double size;


public:

  const DoubleCRT& getDCRT() const { return dcrt; }
  long getPtxtSpace() const { return ptxtSpace; }
  const Context& getContext() const { return dcrt.getContext(); }
  double getSize() const { return size; }

  FatEncodedPtxt_BGV(const EncodedPtxt_BGV& eptxt, const IndexSet& s)
    : dcrt(eptxt.getPoly(), eptxt.getContext(), s), 
      ptxtSpace(eptxt.getPtxtSpace()),
      size(embeddingLargestCoeff(eptxt.getPoly(), eptxt.getContext().zMStar)) 
  { }

};

class FatEncodedPtxt_CKKS {

private:
  DoubleCRT dcrt;
  double mag, scale, err;

public:

  const DoubleCRT& getDCRT() const { return dcrt; }
  double getMag() const { return mag; }
  double getScale() const { return scale; }
  double getErr() const { return err; }
  const Context& getContext() const { return dcrt.getContext(); }

  FatEncodedPtxt_CKKS(const EncodedPtxt_CKKS& eptxt, const IndexSet& s)
    : dcrt(eptxt.getPoly(), eptxt.getContext(), s),
      mag(eptxt.getMag()), scale(eptxt.getScale()), err(eptxt.getErr()) { }

};

class FatEncodedPtxt_base {
public:

  virtual ~FatEncodedPtxt_base() {}

  virtual FatEncodedPtxt_base* clone() const = 0;
  // makes this usable with cloned_ptr

  virtual bool isBGV() const { return false; }
  virtual bool isCKKS() const { return false; }

  virtual const FatEncodedPtxt_BGV& getBGV() const { throw std::bad_cast(); }
  virtual const FatEncodedPtxt_CKKS& getCKKS() const { throw std::bad_cast(); }

};

class FatEncodedPtxt_derived_BGV : public FatEncodedPtxt_base, 
                                public FatEncodedPtxt_BGV {
public:

  virtual FatEncodedPtxt_base* clone() const override 
  { return new FatEncodedPtxt_derived_BGV(*this); }

  virtual bool isBGV() const override { return true; }

  virtual const FatEncodedPtxt_BGV& getBGV() const override { return *this; }

  using FatEncodedPtxt_BGV::FatEncodedPtxt_BGV;
};


class FatEncodedPtxt_derived_CKKS : public FatEncodedPtxt_base, 
                                public FatEncodedPtxt_CKKS {
public:

  virtual FatEncodedPtxt_base* clone() const override 
  { return new FatEncodedPtxt_derived_CKKS(*this); }

  virtual bool isCKKS() const override { return true; }

  virtual const FatEncodedPtxt_CKKS& getCKKS() const override { return *this; }

  using FatEncodedPtxt_CKKS::FatEncodedPtxt_CKKS;
};


/*

Usage:

  FatEncryptedPtxt feptxt;
  EncryptedPtxt eptxt;
  IndexSet s;

  feptxt.expand(eptxt, s);
  // sets feptxt to an expanded (DCRT) version of eptxt
  // withthe given IndexSet

  feptxt.reset(); 
  // empties out feptxt

  // Also supports methods isBGV(), isCKKS(), getBGV(), and getCKKS(),
  // analogous to EncodedPtxt.

*/

class FatEncodedPtxt {
  cloned_ptr<FatEncodedPtxt_base> rep;

public:

  bool isBGV() const { return !rep.null() && rep->isBGV(); }
  bool isCKKS() const { return !rep.null() && rep->isCKKS(); }

  const FatEncodedPtxt_BGV& getBGV() const 
  { if (rep.null()) throw std::bad_cast(); return rep->getBGV(); }

  const FatEncodedPtxt_CKKS& getCKKS() const 
  { if (rep.null()) throw std::bad_cast(); return rep->getCKKS(); }

  void expand(const EncodedPtxt& eptxt, const IndexSet& s)
  {
    if (eptxt.isBGV())
      rep.set_ptr(new FatEncodedPtxt_derived_BGV(eptxt.getBGV(), s));
    else if (eptxt.isCKKS())
      rep.set_ptr(new FatEncodedPtxt_derived_CKKS(eptxt.getCKKS(), s));
    else
      rep.set_ptr(0);
  }

  void reset() 
  {
    rep.set_ptr(0);
  }

};


}

#endif
