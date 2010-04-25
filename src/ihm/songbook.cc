#include "songbook.hh"

#include <QFile>
#include <QTextStream>

#include <iostream>

CSongbook::CSongbook()
  : QObject()
  , m_songs()
{}

CSongbook::~CSongbook()
{}

void CSongbook::setSongs( QStringList & songs )
{
  m_songs = songs;
}

QStringList CSongbook::getSongs()
{
  return m_songs;
}

void CSongbook::save( QString & filename )
{
  QFile file( filename );
  if( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
      QTextStream out( &file );
      out << ( m_songs.join("\n") );
      file.close();
    }
  else
    {
      std::cerr << "unable to open file in write mode" << std::endl;
    }
}

void CSongbook::load( QString & filename )
{
  QFile file( filename );
  if( file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      QTextStream in( &file );
      m_songs = in.readAll().split("\n", QString::SkipEmptyParts);
      file.close();
    }
  else
    {
      std::cerr << "unable to open file in read mode" << std::endl;
    } 
}
