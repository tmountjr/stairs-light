<template>
  <div id="upload-sketch">
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
</template>

<style scoped>
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
        await axios.post('/update', formData, {
          headers: {
            'Content-Type': 'multipart/form-data'
          },
          onUploadProgress: function(progressEvent) {
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
