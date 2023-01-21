#pragma once
#include "common.hh"
#include <vector>

using ActionOnSize = Size(Size);
using ActionOnPos  = Pos (Size,Pos);
struct Action {
	ActionOnSize* size;
	ActionOnPos*  pos;
};


struct Group {
	std::size_t size;
	std::vector<Action> elements;
	std::vector<Action> inverses;

	// OPT: make a constructor to assert elements & inverses
	//      are set with the same size

	const Action& operator[](signed index) const {
		if (index >= 0)  { return elements[ index];  }
		else             { return inverses[-index]; }
	}
};


/* ALL Possible Symmetry Elements Of A Polyomino */
namespace SymmetryAction {
	#define Make_Action(NAME,I,J,ORIENT) \
	Action NAME { \
		/*size:*/ \
		[](Size s) -> Size { \
			unsigned m=s.m, n=s.n; \
			/**/ if constexpr (ORIENT == 0) { return {m,n}; } \
			else if constexpr (ORIENT == 1) { return {n,m}; } \
		} , \
		/*pos:*/ \
		[](Size s, Pos p) -> Pos { \
			[[maybe_unused]] \
			unsigned m=s.m-1, n=s.n-1, i=p.i, j=p.j; \
			return {I,J}; \
		} \
	}

	Make_Action(Identity,   i,   j, 0);
	Make_Action(Rot90   , n-j,   i, 1);
	Make_Action(Rot180  , m-i, n-j, 0);
	Make_Action(Rot270  ,   j, m-i, 1);
	Make_Action(Flip    ,   i, n-j, 0);
	Make_Action(Flip90  ,   j,   i, 1);
	Make_Action(Flip180 , m-i,   j, 0);
	Make_Action(Flip270 , n-j, m-i, 1);

	#undef Make_Action
}

/* All Polyomino Symmetry Groups */
namespace Symmetry {
	using namespace SymmetryAction;

	/* no symmetries; all actions are unique shapes */
	Group None { 8 ,
		// elems:
		{Identity, Rot90   , Rot180  , Rot270  ,
		 Flip    , Flip90  , Flip180 , Flip270 } ,
		// inv:
		{Identity, Rot270  , Rot180  , Rot90   ,
		 Flip    , Flip90  , Flip180 , Flip270 }
	};

	/* symmetry about a single axis (gridline or diagonal) */
	Group Mirror1 { 4 ,
		// elems:
		{Identity, Rot90   , Rot180  , Rot270  } ,
		// inv:
		{Identity, Rot270  , Rot180  , Rot90   }
	};

	/* S-curve Symmetry */
	Group Rotation2 { 4 ,
		// elems:
		{Identity, Rot90   , Flip    , Flip90  } ,
		// inv: 
		{Identity, Rot90   , Flip    , Flip90  }
	};

	/* symmetry about two axies (grid or diag.) */
	Group Mirror2 { 2 ,
		// elems:
		{Identity, Rot90   } ,
		// inv:
		{Identity, Rot90   }
	};

	/* german symmetry */
	Group Rotation4 { 2 ,
		// elems:
		{Identity, Flip    } ,
		// inv:
		{Identity, Flip    }
	};

	/* the polyomino has every symmetry; there is only 1 unique shape */
	Group Square { 1 ,
		// elems:
		{Identity} ,
		// inv:
		{Identity}
	};
}