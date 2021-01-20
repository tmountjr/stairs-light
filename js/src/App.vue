<template>
  <div id="upload-sketch">
    <header>
      <div class="home-menu pure-menu pure-menu-horizontal pure-menu-fixed">
        <a class="pure-menu-heading">Upload Sketch</a>
        <ul class="pure-menu-list">
          <li class="pure-menu-item pure-menu-disabled">Current Version: 1.2</li>
        </ul>
      </div>
    </header>
    <div class="wrapper pure-g">
      <div class="content pure-u-1-3">
        <form class="pure-form">
          <fieldset>
            <legend>Upload the .bin file:</legend>
            <label class="pure-button">
              Browse...
              <input type="file" name="upload" hidden @change="fileChange($event.target.files)">
            </label>
            <input
              type="button"
              value="Upload"
              class="pure-button pure-button-primary"
              :disabled="!uploadReady"
              @click="upload"
            >
          </fieldset>
        </form>
        <progress v-if="showUploadMeter" max="100" :value.prop="uploadPercentage"></progress>
      </div>
    </div>
  </div>
</template>

<style scoped>
  header {
    height: 2.5em;
  }
  
  .home-menu {
    box-shadow: 0 1px 1px rgba(0, 0, 0, .1);
    background: #2d3e50;
    display: flex;
    justify-content: space-between;
  }

  .home-menu .pure-menu-heading {
    color: #eee;
    font-weight: 400;
    font-size: 120%;
  }

  .home-menu .pure-menu-disabled {
    color: #eee;
    font-weight: 400;
  }

  .home-menu .pure-menu-list {
    height: 100%;
  }

  .wrapper {
    padding-top: 2em;
  }

  .content {
    margin: 0 auto;
  }

  .pure-form label.pure-button {
    margin: 0;
  }

  progress[value] {
    appearance: none;
    border: none;
    width: 100%;
    height: 20px;
  }

  progress[value]::-webkit-progress-bar {
    background-color: #eee;
    border-radius: 2px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.25) inset;
  }

  progress[value]::-webkit-progress-value {
      background-image: 
        -webkit-linear-gradient(-45deg, transparent 33%, rgba(0, 0, 0, .1) 33%, rgba(0,0, 0, .1) 66%, transparent 66%),
        -webkit-linear-gradient(top,  rgba(255, 255, 255, .25), rgba(0, 0, 0, .25)),
        -webkit-linear-gradient(left, #0078e7, rgb(202, 60, 60));

    border-radius: 2px;
    background-size: 35px 20px, 100% 100%, 100% 100%;
  }

  progress[value]::-moz-progress-bar {
    background-image:
      -moz-linear-gradient(135deg, transparent 33%, rgba(0, 0, 0, 0.1) 33%, rgba(0, 0, 0, 0.1) 66%, transparent 66%),
      -moz-linear-gradient(top, rgba(255, 255, 255, 0.25), rgba(0, 0, 0, 0.25)),
      -moz-linear-gradient(left, #0078e7, rgb(202, 60, 60));

    border-radius: 2px;
    background-size: 35px 20px, 100% 100%, 100% 100%;
  }
</style>

<script>
import * as axios from 'axios'

export default {
  name: 'upload-sketch',
  data() {
    return {
      toUpload: null,
      uploadPercentage: 0,
      showUploadMeter: false,
    }
  },
  methods: {
    fileChange: function(formData) {
      this.toUpload = formData[0]
    },
    upload: async function() {
      this.showUploadMeter = true;
      let formData = new FormData()
      formData.append('file', this.toUpload)
      try {
        const resp = await axios.post('/update', formData, {
          headers: {
            'Content-Type': 'multipart/form-data'
          },
          onUploadProgress: function (progressEvent) {
            this.uploadPercentage = parseInt(Math.round(progressEvent.loaded / progressEvent.total) * 100)
          }.bind(this)
        })
        console.log('Success!')
      } catch (e) {
        console.log('Failure!')
        console.log(e)
      }
    },
  },
  computed: {
    uploadReady: function() {
      return this.toUpload !== null
    }
  }
}
</script>
