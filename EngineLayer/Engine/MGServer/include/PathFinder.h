#ifndef PathFinder_H
#define PathFinder_H

#pragma once

#include <windows.h>
#include <list>

struct Position3D
{
public:
	float x;
	float y;
	float z;

	Position3D() : x(0.0f), y(0.0f), z(0.0f)
	{
	}

	// stl allocator에선 복사생성자는 const를 사용하는 복사생성자를 정의해야한다.
	Position3D(const Position3D& right) : x(right.x), y(right.y), z(right.z)
	{
	}

	Position3D(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	bool operator==(Position3D& right)
	{
		// 제대로 할려면 float문제점을 생각한 연산을 해야한다.
		if( x == right.x && y == right.y && z == right.z ) return true;
		return false;
	}
};

typedef std::list< Position3D > tPosition3DLIST;
typedef std::list< Position3D >::iterator tPosition3DLISTITER;



class IGraphManager
{
public:
	virtual long calcHueristic(Position3D& p1, Position3D& p2) = 0;
	virtual DWORD getUniqueNumber(Position3D& p) = 0;
	virtual void gatherNeighborNodes(Position3D& base, tPosition3DLIST& output) = 0;
};

class PathFinder
{
protected:


	struct NODEDESC
	{
		long		s_f,s_h;
		DWORD		s_dw_g,s_dw_tmpg;
		Position3D	s_pos;
		DWORD		s_dw_NodeNum;
	};


	struct NODE : public NODEDESC
	{
		
		NODE*	s_pParent;
		NODE*	s_Child[8];
		

		NODE()
		{
			ZeroMemory(&s_f, sizeof(NODE));
		}		
	};

	typedef std::list< NODE* > tNODELIST;
	typedef std::list< NODE* >::iterator tNODELISTITER;
	typedef std::list< NODE* >::reverse_iterator tNODELISTRVITER;



	tNODELIST	openList;
	tNODELIST	closeList;
	tNODELIST	usedList;

	tNODELIST	travTreeList;	

	IGraphManager* instantGraphManager;

protected:

	static bool PathLess (NODE* elem1, NODE* elem2 )
	{
		return elem1->s_f < elem2->s_f;
	}

	void Finit();

	NODE* ReturnBestNode();

	void GenerateSuccessors(NODE* BestNode, Position3D& end);
	void GenerateSucc(NODE* BestNode, Position3D& childpos, Position3D& end);

	void Insert(NODE* Successor);

	NODE* CheckOpen(DWORD nodeNum);
	NODE* CheckClosed(DWORD nodeNum);

	void PropagateDown(NODE* Old);

	void Push(NODE *Node);
	NODE* Pop();

public:
	PathFinder();
	virtual ~PathFinder();

	BOOL findPath(Position3D& start, Position3D& end, IGraphManager* igm, std::list<DWORD>& output);
};

#endif
