// Copyright (c) 2010-2020, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license. We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.
#ifndef MFEM_XFL_HPP
#define MFEM_XFL_HPP

#include <list>
#include <array>
#include <string>
#include <memory>
#include <unordered_map>

// *****************************************************************************
/** @cond */ // Doxygen warning Detected potential recursive class relation
template <typename T, typename... Types>
struct Atom : public Atom<T>, public Atom<Types...>
{
   using Atom<T>::Accept;
   using Atom<Types...>::Accept;
};
template <typename T> struct Atom<T>
{ virtual void Accept(T&, const bool = true) = 0; };

template <typename T, typename... Types>
struct Middlends : public Middlends<T>, public Middlends<Types...>
{
   using Middlends<T>::Visit;
   using Middlends<Types...>::Visit;
};
template <typename T> struct Middlends<T> { virtual void Visit(T&) = 0; };
/** @endcond */

// *****************************************************************************
class xfl;
class Debug;
struct Node;
struct Rule;
struct Token;
namespace yy { class location; }

// *****************************************************************************
struct Middlend /** @cond */: public Middlends<Rule, Token> /** @endcond */
{
   xfl &ufl;
   Middlend(xfl &ufl) : ufl(ufl) { }
};

// *****************************************************************************
struct Node: public Atom<Middlend>
{
   int n;
   std::string name;
   int id {0}, nnext {0};
   Node *next {nullptr}, *child {nullptr}, *root {nullptr};
   struct {bool down;} dfs {true};
   Node(int n, const char *name): n(n), name(name) {}
   const int Number() const { return n; }
   const std::string Name() const { return name; }
   const char *CStr() const { return name.c_str(); }
   virtual void Accept(Middlend&, const bool down = true) = 0;
   virtual const bool IsRule() const = 0;
   virtual const bool IsToken() const = 0;
   virtual ~Node() {}
};

// *****************************************************************************
struct Rule : public Node
{
   Rule(int rn, const char *name): Node(rn, name) {}
   void Accept(Middlend &me, const bool down = true)
   { dfs.down = down; me.Visit(*this); }
   const bool IsRule() const { return true; }
   const bool IsToken() const { return false; }
};

// *****************************************************************************
struct Token : public Node
{
   Token(int tk, const char *name): Node(tk, name) {}
   void Accept(Middlend &me, const bool = true)
   { dfs.down = true; me.Visit(*this); }
   const bool IsRule() const { return false; }
   const bool IsToken() const { return true; }
};

// *****************************************************************************
class xfl
{
   using Node_sptr = std::shared_ptr<Node>;
public:
   Node *root;
   yy::location *loc;
   bool yy_debug;
   bool ll_debug;
   std::string &input, &output;
   struct { int debug; bool echo; } ll;
   enum VarMode: unsigned
   {
      NONE   = 0,
      INTERP = 1 << 0,
      GRAD   = 1 << 1,
      DIV    = 1 << 2,
      CURL   = 1 << 3
   };
   // Variables
   struct var
   {
      const std::string name;
      int type {-1};
      unsigned mode { xfl::NONE };
      var(const std::string n, int t, VarMode m): name(n), type(t), mode(m) {}
   };
   // Context
   struct
   {
      int type;
      Node *qfunc { nullptr };
      Node *extra { nullptr };
      std::unordered_map<std::string, xfl::var> vars;
   } ctx;
public:
   xfl(bool yy_debug, bool ll_debug, std::string &input, std::string &output);
   ~xfl();
   int open();
   int close();
   int parse(const std::string&, std::ostream&);
   int morph(std::ostream&);
   int code(std::ostream&);
   Node* &Root() { return root;}
public:
   Node *astAddNode(Node_sptr);
   void dfs(Node*, Middlend&);
   bool HitRule(const int, Node*);
   bool HitToken(const int, Node*);
   bool OnlyToken(const int, Node*);
   Node *GetToken(const int, Node*);
};

// *****************************************************************************
#define DBG(...) { printf("\033[32m");  \
                   printf(__VA_ARGS__); \
                   printf(" \n\033[m"); \
                   fflush(0); }

// *****************************************************************************
#include <string>
#include <cstring>
#include <iomanip>
#include <iostream>

class Debug
{
   const bool debug = false;
public:
   inline Debug() {}

   inline Debug(const int mpi_rank,
                const char *FILE, const int LINE,
                const char *FUNC, int COLOR): debug(true)
   {
      if (!debug) { return; }
      const char *base = Strrnchr(FILE,'/', 2);
      const char *file = base ? base + 1 : FILE;
      const uint8_t color = COLOR ? COLOR : 20 + Checksum8(FILE) % 210;
      std::cout << "\n\033[38;5;" << std::to_string(color) << "m";
      std::cout << file << ":";
      std::cout << "\033[2m" << std::setw(4) << LINE << "\033[22m: ";
      if (FUNC) { std::cout << "[" << FUNC << "] "; }
      std::cout << "\033[1m";
   }

   ~Debug()
   {
      if (!debug) { return; }
      std::cout << "\033[m";
      std::cout << std::endl;
   }

   template <typename T>
   inline void operator<<(const T &arg) const noexcept { std::cout << arg; }

   template<typename T, typename... Args>
   inline void operator()(const char *fmt, const T &arg,
                          Args... args) const noexcept
   {
      if (!debug) { return; }
      for (; *fmt != '\0'; fmt++ )
      {
         if (*fmt == '%')
         {
            fmt++;
            const char c = *fmt;
            if (c == 'p') { operator<<(arg); }
            if (c == 's' || c == 'd' || c == 'f') { operator<<(arg); }
            if (c == 'x' || c == 'X')
            {
               std::cout << std::hex;
               if (c == 'X') { std::cout << std::uppercase; }
               operator<<(arg);
               std::cout << std::nouppercase << std::dec;
            }
            if (c == '.')
            {
               fmt++;
               const char c = *fmt;
               char num[8] = { 0 };
               for (int k = 0; *fmt != '\0'; fmt++, k++)
               {
                  if (*fmt == 'e' || *fmt == 'f') { break; }
                  if (*fmt < 0x30 || *fmt > 0x39) { break; }
                  num[k] = *fmt;
               }
               const int fx = std::atoi(num);
               if (c == 'e') { std::cout << std::scientific; }
               if (c == 'f') { std::cout << std::fixed; }
               std::cout << std::setprecision(fx);
               operator<<(arg);
               std::cout << std::setprecision(6);
            }
            return operator()(fmt + 1, args...);
         }
         operator<<(*fmt);
      }
   }

   template<typename T>
   inline void operator()(const T &arg) const noexcept
   {
      if (!debug) { return; }
      operator<<(arg);
   }

   inline void operator()() const noexcept { }

public:
   static const Debug Set(const char *FILE, const int LINE, const char *FUNC,
                          int COLOR = 0)
   {
      static int mpi_dbg = 0, mpi_rank = 0;
      static bool env_mpi = false, env_dbg = false;
      static bool ini_dbg = false;
      if (!ini_dbg)
      {
         const char *DBG = getenv("DBG");
         const char *MPI = nullptr;
         env_dbg = DBG != nullptr;
         env_mpi = MPI != nullptr;
#ifdef MFEM_USE_MPI
         int mpi_ini = false;
         MPI_Initialized(&mpi_ini);
         if (mpi_ini) { MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank); }
         mpi_dbg = atoi(env_mpi ? MPI : "0");
#endif
         ini_dbg = true;
      }
      const bool debug = (env_dbg && (!env_mpi || mpi_rank == mpi_dbg));
      return debug ? Debug(mpi_rank, FILE, LINE, FUNC, COLOR) : Debug();
   }

private:
   inline uint8_t Checksum8(const char *bfr)
   {
      unsigned int chk = 0;
      size_t len = strlen(bfr);
      for (; len; len--,bfr++) { chk += static_cast<unsigned int>(*bfr); }
      return (uint8_t) chk;
   }

   inline const char *Strrnchr(const char *s, const unsigned char c, int n)
   {
      size_t len = strlen(s);
      char *p = const_cast<char*>(s) + len - 1;
      for (; n; n--,p--,len--)
      {
         for (; len; p--,len--)
            if (*p == c) { break; }
         if (!len) { return nullptr; }
         if (n == 1) { return p; }
      }
      return nullptr;
   }

};

#ifndef MFEM_DEBUG_COLOR
#define MFEM_DEBUG_COLOR 0
#endif

#define dbg(...) \
    Debug::Set(__FILE__,__LINE__,__FUNCTION__,MFEM_DEBUG_COLOR).\
    operator()(__VA_ARGS__)

#endif // MFEM_XFL_HPP
