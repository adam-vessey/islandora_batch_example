<?php

class RSSBatchPreprocessor extends IslandoraBatchPreprocessor {
  public function preprocess() {
    $mod_path = drupal_get_path('module', 'islandora_batch_example');

    // Load up our RSS.
    $rss = simplexml_load_file("$mod_path/rss.xml");

    // Accumulate a list of the objects added to the queue...
    $added = array();

    // Now, we iterate over each item in the RSS...
    foreach ($rss->xpath('/rss/channel/item') as $item) {
      // ... instanciate our object we started defining above...
      $batch_object = new RSSBatchObject($this->connection, $item);

      // ... and add throw the instances into the queue.
      $this->addToDatabase($batch_object);
      $added[] = $batch_object;
    }

    // Return the list... Optional really, but useful if you want to preprocess
    // something and ingest it right away. This will likely change in the near
    // future, so sets of preprocessed items can be identified more easily,
    // instead of passing around instances of objects.
    return $added;
  }
}
