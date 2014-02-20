<?php

class RSSBatchObject extends IslandoraBatchObject {
  protected $rss_item_xml;

  public function __construct(IslandoraTuque $connection, SimpleXMLElement $rss_item) {
    // Need the connection to perform the other setup for the Tuque object.
    parent::__construct(NULL, $connection->repository);

    // We accept SimpleXMLElements to simplify our interface. We cannot store
    // the element as it is natively, as SimpleXMLElements do not appear to
    // support serialization, which happens during the batch process.
    $this->rss_item_xml = $rss_item->asXML();
  }

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
    $mods->label = $title;
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
