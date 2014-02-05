import jnicli.*;

import java.util.*;

class Word {
    int sentence;
    String text;
    static final String CONSTRAINTS = "sentence references Sentence(words), text using index";
}
    
class Sentence { 
    int[] words;
    static final String CONSTRAINTS = "words references Word(sentence)";
}


public class JniTestRelations { 
    final static int nWords = 1000;
    final static int nSentences = 10000;
    final static int maxWordsPerSentense = 10;
    final static int initSize = 8*1024*1024; // 40Mb page pool

    static public void main(String[] args) throws java.io.IOException {    
        Database db = new DatabaseJNI();
        db.open(Database.READ_WRITE, "text", "text.dbs", initSize, 0);
        long start = System.currentTimeMillis();
        long key = 1999;
        for (int i = 0; i < nSentences; i++) { 
            key = (3141592621L*key + 2718281829L) % 1000000007L;
            Sentence s = new Sentence();
            int sid = (int)db.insert(s);
            int nWordsInSentence = (int)(key % maxWordsPerSentense);
            for (int j = 0; j < nWordsInSentence; j++) { 
                key = (3141592621L*key + 2718281829L) % 1000000007L;
                Word word = new Word();
                word.sentence = sid;
                word.text = Long.toString(key % nWords);
                db.insert(word);
            }
        }
        db.commit();
        System.out.println("Elapsed time for inserting " + nSentences + " records: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        
        start = System.currentTimeMillis();
        int totalSentences = 0;
        for (int i = 0; i < nWords; i++) { 
            Cursor sentences = db.select(Sentence.class, "exists i:(words[i].text='" + i + "')", 0);
            totalSentences += sentences.size();
          SentenceLoop:
            while (sentences.hasMoreElements()) { 
                Sentence s = (Sentence)sentences.nextElement();
                for (int j = 0; j < s.words.length; j++) { 
                    Cursor words = db.select(Word.class, "current=" + s.words[j], 0);
                    Word word = (Word)words.nextElement();
                    if (word.text.equals(Integer.toString(i))) { 
                        continue SentenceLoop;
                    }
                }
                throw new Error("Word is not found");
            }
        }
        System.out.println("Elapsed time for performing " + nWords + " word searches: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        db.close();
    }
}
