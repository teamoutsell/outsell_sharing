Outsell Sharing
===============

This is a Drupal 7 module that handles Outsell content sharing between sites with this
module installed. Specifically it:

  1. Shares `news`, `insights` and `reports`. (this is extendable)
  2. It allows sites to either send or recieve content
  3. It sets up secure API key based communication between the sites
  4. Handles legacy `tethys` content and redirects eg `search/d7entity` paths

Getting Started
---------------

Clone the project and install dependencies to get ready for development

```bash
git clone git@github.com:teamoutsell/outsell_sharing.git
cd outsell_sharing
composer install
```

Testing
-------

```bash
composer test
```

Configuration
-------------

There are config options available at `admin/config/outsell/sharing`. If you are using [platform.sh](http://platform.sh) you can set these options as [platform environmental variables](https://docs.platform.sh/development/variables.html#drupal-specific-variables) and this module will use and enforce those values.

If you **are** using platform.sh and kalabox make sure you `kbox pullenv && kbox restart` to get any relevant config that has already been set up for you.

### Receiving content

In the configuration page above you will want to select *Allow this site to get content from other sites*. This will trigger an additional set of *Configure Getting
* options. Select the content types you want to receive and then save the form.

**NOTE:** If you plan on sending content from another site to this site, copy the API key since you will need this.

### Sending content

In the configuration page above you will want to select *Allow this site to send content to other sites.*. This will trigger an additional set of *Configure Sending
* options.

Enter a list of sites you want to send content to along with the API keys for those sites. These sites must also have installed this module and have API keys configured. See *Recieving Content* above on how to get the API key. Here is an example:

```
https://my.awesome.site|myawesomeapikey
https://another.site|0932ufin3opf23
```

**NOTE:** It is **highly** recommended that you send content over `https`.

Next, select the kinds of content you want to send and save the form.

### Batch sending content

In the configuration page above you will want to select *Allow this site to send content to other sites.*. This will trigger an additional set of *Batch send content* options.

Click on *Send Content* to begin transfering content to the sites specified. If you want to test or debug your content delivery you can limit the amount of content to send, the sorting of that content and whether to display any failed payload if the content does not send correctly.

Local Setup with Kalabox
------------------------

By default Kalabox is not great with multi-site communication so you will need to do a few extra things to get sharing to work locally:

  1. Start all the "receiving sites" first.
  2. Run `docker inspect SITENAME_web_1 | grep IPAddress` on each receiving site and make a note of the IP for each site.
  3. Open up the `kalabox-compose.yml` for each "sending site" and add the following lines to the `appserver` service based on what you got back in #2.

    ```
    extra_hosts:
    - "site1.kbox.site:172.17.0.10"
    - "site2.kbox.site:172.17.0.12"
    ```

  4. Restart each sending site.
  5. profit

Debugging
---------

If some content is not sending correctly the best way to debug it is to enable the debug mode from the config page.

1. Go to the config page from a "sending site".
2. Select the *Debug content that failed to send* option in the *Batch send content* section.
3. Select any limits or sorting you want.
4. Click *Send Content*

When the content has finished sending you will see JSON objects of any failed payloads. You can use this object directly with something like [Postman](https://chrome.google.com/webstore/detail/postman/fhbjgbiflinjbdggehcddcbncdddomop?hl=en), [DHC](https://chrome.google.com/webstore/detail/dhc-restlet-client/aejoelaoggembcahagimdiliamlcdmfm?hl=en) or cURL combined with [xdebug](https://xdebug.org/) to see what is going wrong. Here is some example `DHC` config to help you set up your own client

```json
{
  "front-version": "1.4.2.1",
  "version": 3,
  "nodes": [{
    "id": "B97D8272-8F61-43D4-8FD8-0D49F7E40EC2",
    "lastModified": "2017-02-08T23:13:23.714-08:00",
    "name": "OUTSELL SHARING POST",
    "headers": [{
      "enabled": true,
      "name": "Authorization",
      "value": "YOUR_API_KEY_GOES_HERE"
    }, {
      "enabled": true,
      "name": "Content-Type",
      "value": "application/json"
    }],
    "metaInfo": {
      "ownerId": "Local repository id"
    },
    "type": "Request",
    "method": {
      "requestBody": true,
      "link": "http://tools.ietf.org/html/rfc7231#section-4.3.3",
      "name": "POST"
    },
    "body": {
      "autoSetLength": true,
      "textBodyEditorHeight": 454,
      "textBody": "YOUR_JSON_DATA_GOES_HERE",
      "formBody": {
        "overrideContentType": true,
        "encoding": "application/x-www-form-urlencoded",
        "items": [{
          "enabled": true,
          "name": "data",
          "value": "ENCODEDDATA",
          "type": "Text"
        }]
      },
      "bodyType": "Text"
    },
    "headersType": "Form",
    "uri": {
      "host": "YOUR_SITE_GOES_HERE",
      "path": "/outsell/sharing/api/share",
      "query": {
        "delimiter": "&",
        "items": [{
          "enabled": true,
          "name": "XDEBUG_SESSION_START",
          "value": "sublime.xdebug"
        }]
      },
      "scheme": {
        "secure": true,
        "name": "https",
        "version": "V11"
      }
    }
  }]
}
```

**NOTE:** If you are testing locally over `https` you may need to open up your browser and visit your endpoint to white list any self signed certs as needed.
