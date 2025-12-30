# Smart Music Player
This is the Semester project of Our DSA Project. Its An Music Player that handles playing songs, playlists and recommend songs and searching songs.

## Libraries Used
1. TGUI for Widgets and Panels
2. SFML for Displaying Windows
3. libmpg3 for MP3 playback
4. An self made UI_template.h to make things easier

## Concepts Used
1. Cosine Similarity to Calculate similarity between two songs
2. Graph to link similar songs
3. Trie or Prefix Tree for Searching Effceintly
4. Maps to Access Every Song in O(1) complexity
5. An Doubly Linked List as an playlist that has each node as an song 

## WorkFlow

### 1. When Program is Started
files/final_songs_set.csv contains metadata about all the songs that is loaded when the program is started from which these things happen
1. An map that store each song id as an key and struct to that song as an value.
2. An Recommendation Graph is made by Calculating similarity.
3. An Playlist According to each genre
4. An Playlist According to each artist
     
files/user_playlists.csv contains playlists that are made by user when program is started this file is also loaded.

### 2. How an Song is Played
An folder must be added songs/ that have all the songs that present in files/user_playlists.csv the name of the songs must be thier id so that when we get an song id from map in O(1) we can also get that song file in O(1) by same id.

### 3. How Playlist is Played
There is Doubly Linked List that contains each song id that are in playlist and it has an head tail and curr pointer so when user play next song it goes to curr->next and when user hit previous button it goes to curr->prev andd curr song id is return and played.

### 4. How User Playlists Handled
when user add an playlist its writed to files/user_playlists.csv

### 5. How Smart Play Works
Similarity of each song with every other song is calculated . if similarity  > 0.5 put that song in graph. when user clicks on smart play button the program checks the current song playing and goes to graph and return all the songs adjacent to current song return in vector and song with most similairty is added to playlist and when user clicks again the 2nd most similar song is suggested.

