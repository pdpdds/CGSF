// -*- Mode: C++ -*-

//          GiSTfile.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTfile.cpp,v 1.1 2007/07/18 20:45:49 knizhnik Exp $

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#ifdef UNIX
#include <unistd.h>
#else
#include <io.h>
#endif

#ifdef UNIX
#define O_BINARY 0
#endif

#include "GiSTfile.h"

// The first page in the file has these "magic words"
// and the head of the deleted page list.
static char magic[] = "GiST data file";

void 
GiSTfile::Create(const char *filename)
{
  if (IsOpen())
    return;

  fileHandle = open(filename, O_RDWR | O_BINARY);
  if (fileHandle >= 0) {
	close(fileHandle);
    return;
  }

  fileHandle = open(filename,
					O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
					S_IREAD | S_IWRITE);

  if (fileHandle < 0)
    return;

  SetOpen(1);

  /* Reserve page 0 */
  char *page = new char[PageSize()];
  memset(page, 0, PageSize());
  memcpy(page, magic, sizeof magic);
  write(fileHandle, page, PageSize());
  delete page;
}

void 
GiSTfile::Open(const char *filename)
{
  char *page;

  if (IsOpen())
    return;

  fileHandle = open(filename, O_RDWR | O_BINARY);

  if (fileHandle < 0)
    return;

  // Verify that the magic words are there
  page = new char[PageSize()];
  read(fileHandle, page, PageSize());
  if (memcmp(page, magic, sizeof(magic))) {
    close(fileHandle);
	delete page;
    return;
  }
  delete page;
  SetOpen(1);
}

void 
GiSTfile::Close()
{
  if (!IsOpen())
    return;

  close(fileHandle);
  SetOpen(0);
}

void 
GiSTfile::Read(GiSTpage page, char *buf)
{
  if (IsOpen()) {
    lseek(fileHandle, page * PageSize(), SEEK_SET);
    read(fileHandle, buf, PageSize());
  }
}

void 
GiSTfile::Write(GiSTpage page, const char *buf)
{
  if (IsOpen()) {
    lseek(fileHandle, page * PageSize(), SEEK_SET);
    write(fileHandle, buf, PageSize());
  }
}

GiSTpage 
GiSTfile::Allocate()
{
  GiSTpage page;
  char *buf;

  if (!IsOpen())
    return (0);

  // See if there's a deleted page
  buf = new char[PageSize()];
  Read(0, buf);
  memcpy(&page, buf+sizeof(magic), sizeof(GiSTpage));

  if (page) {
    // Reclaim this page
    Read(page, buf);
    Write(0, buf);
  } else {
    page = lseek(fileHandle, 0, SEEK_END) / PageSize();
    memset(buf, 0, PageSize());
    write(fileHandle, buf, PageSize());
  }
  delete buf;
  return page;
}

void 
GiSTfile::Deallocate(GiSTpage page)
{
  char *buf;
  GiSTpage temp;

  if (!IsOpen())
    return;

  // Get the old head of the list
  buf = new char[PageSize()];
  Read(0, buf);
  memcpy(&temp, buf+sizeof(magic), sizeof(GiSTpage));

  // Write the new head of the list
  memcpy(buf+sizeof(magic), &page, sizeof(GiSTpage));
  Write(0, buf);

  // In our new head, put link to old head
  memcpy(buf+sizeof(magic), &temp, sizeof(GiSTpage));
  Write(page, buf);
  delete buf;
}








