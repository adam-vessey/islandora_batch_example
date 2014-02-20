A simpler case for an ingest might be to take an RSS feed and create objects for each entry.

First, our RSS feed to work on:
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<rss version="2.0">
  <channel>
    <title>RSS Example</title>
    <link>http://example.com</link>
    <description>An exemplary example</description>
    <item>
      <title>First item</title>
      <link>http://example.com/first</link>
      <description>The first example entry</description>
    </item>
    <item>
      <title>Second item</title>
      <link>http://example.com/second</link>
      <description>The second example entry</description>
    </item>
  </channel>
</rss>
```

Let us start implementing our `IslandoraBatchObject` subclass. For
preprocessing, all that really matters is creating an instance of the object
and making :
```php
RSSBatchObject extends IslandoraBatchObject {
  protected $rss_item_xml;

  public function __construct(IslandoraTuque $connection, SimpleXMLElement $rss_item) {
    // Need the connection to perform the other setup for the Tuque object.
    parent::__construct(NULL, $connection->repository);

    // We accept SimpleXMLElements to simplify our interface. We cannot store
    // the element as it is natively, as SimpleXMLElements do not appear to
    // support serialization, which happens during the batch process.
    $this->rss_item_xml = $rss_item->asXML();
  }

  [...]
}
```

A preprocessor to "feed" this structure is fairly simple:
```php
RSSBatchPreprocessor extends IslandoraBatchPreprocessor {
  public function preprocess() {
    // Load up our RSS.
    $rss = simplexml_load_file('rss.xml');

    // Grab a Tuque connection to reuse. NOTE: Because this is connected to the
    // current user, we have a side effect that the object's owner will be set
    // to the current user.
    $tuque = islandora_get_tuque_connection();

    // Accumulate a list of the objects added to the queue...
    $added = array();

    // Now, we iterate over each item in the RSS...
    foreach ($rss->xpath('/rss/channel/item') as $item) {
      // ... instanciate our object we started defining above...
      $batch_object = new RSSBatchObject($tuque, $item);

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
```

Let us finish implementing `RSSBatchObject`:
```php
RSSBatchObject extends IslandoraBatchObject {
  [...]

  // Responsible for pre-ingest transformations into base datastreams.
  public function batchProcess() {
    // Parse the XML of the item.
    $simple_xml = new SimpleXMLElement($this->rss_item_xml);

    // Grab things from the XML into variables, as strings.
    $this->label = $title = (string) $simple_xml->title;
    $description = (string) $simple_xml->description;
    $url = (string) $simple_xml->link;

    // Generate a simple MODS datastream, and add it to the current object;
    // basic Tuque stuff.
    // Typically, the MODS would also be transformed to DC and added, but
    // we're skipping this, due to the simple data.
    $mods = $this->constructDatastream('MODS', 'M');
    $mods->label = $title
    $mods->mimetype = 'text/xml';
    $mods->content = <<<EOQ
<mods xmlns='http://www.loc.gov/mods/v3'>
  <titleInfo>
    <title>{$title}</title>
  </titleInfo>
  <abstract>{$description}</abstract>
  <location>
    <url>{$url}</url>
  </location>
</mods>
EOQ;
    $this->ingestDatastream($mods);

    // Add relationships. This could also be done while preprocessing, if it
    // will complete quickly. Our two little additions is one of the cases
    // where it would be quick, but let's put it here.
    $this->addRelationships();

    // Indicate that this object is ready to actually go into Fedora.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  // Add a couple relationships.
  public function addRelationships() {
    // Let's make this object a citation in the default citation collection, as
    // MODS is all that is really required to make a citation...
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'ir:citationCollection');
    $this->models = 'ir:citationCModel';
  }

  // Get a list of resources.
  public function getResources() {
    // Not really of use here in our case; though, currently needs to be
    // implemented due to the class hierarchy/interface.
    // Typically, your preprocessor would call this, and pass the values as
    // another parameter to the `IslandoraBatchPreprocessor::addToDatabase()`
    // call.
    return array();
  }
}
```

To actually trigger the preprocessing, one just needs to instanciate the
preprocessor class (typically, one might pass additional parameters to it), and
call the `preprocess()` method.

This module was really made to populate the queue, and to run the
batch to actually ingest things when the server is not loaded (likely at
night), by grabbing things off the queue in whatever order they are obtained
when querying. To just process everything in the queue, you could run:

`drush islandora_batch_ingest`

With that said, it /is/ possibly to cause a subset of objects in the queue to
be ingested immediately, with the `ingest_set` property when ingesting;
however, this is currently only possible when triggering the ingest
programmatically.
