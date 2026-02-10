{-
  Visitor Pattern Inversion in Haskell
-}

module TaskExample where

class Task t where
    estimate :: t -> Int
    priority :: t -> Int

data BugFix = BugFix String
data Feature = Feature String Int

instance Task BugFix where
    estimate (BugFix _) = 5
    priority (BugFix _) = 8

instance Task Feature where
    estimate (Feature _ complexity) = complexity
    priority (Feature _ _) = 6